/*
 * stepper.c
 *
 * Driver for stepper motors. Each motor is a 6-wire unipolar model. Each of
 * the 4 coils (per motor) can be driven with a full current (through the big
 * transistor) or a reduced current (through a smaller transistor + 47 Ohm
 * resistor). The reduced current I call 'half current', but it may be less.
 * 
 * The Y motor (pen movement) is controlled through PORTC, and the X motor
 * (mat roller) is controlled through PORTA. Connections are identical.
 * 
 * In addition, the Z coordinate is controlled by two pins: PE2 is used
 * for up/down toggle, and PB6 selects the pressure with a PWM signal.
 * 
 * A small pushbutton is attached to PD1, which is active low when
 * pushed. This button detects when the gray cover on the pen holder is
 * moved all the way to the 'home' position.
 * 
 * Step resolution is about 400 steps/inch. For a 12x12 inch mat, that means
 * there's about 4800x4800 steps of usable space. Coordinate origin is the
 * blade starting point on the mat. A small amount of negative X is allowed
 * to roll the mat out of the machine.
 *
 *
 * This file is part of FreeExpression.
 *
 * https://github.com/thetazzbot/FreeExpression
 *
 * FreeExpression is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2.
 *
 * FreeExpression is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeExpression. If not, see http://www.gnu.org/licenses/.
 *
 */

#include <avr/wdt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdio.h>
#include "stepper.h"
#include "keypad.h"
#include "timer.h"
#include "display.h"

#define MAT_EDGE 250			// distance to roll to load mat 
#define MAX_Y		4800
#define MAX_X		4800



#define HOME	(1 << 1 ) // PD1, attached to 'home' push button
#define PEN	(1 << 2)  // PE2, attached to pen up/down output

#define at_home( ) (!(PIND & HOME))

/* 
 * motor phases: There are 16 different stepper motor phases, using 
 * various combinations of full/half power to create the smallest 
 * possible steps:
 *    
 *   0   1   2   3   4   5   6   7   8   9  10   11  12  13  14  15 
 *  ___________    .   .   .   .   .   .   .   .   .   .   .________
 *     .   .   \___    .   .   .   .   .   .   .   .    ___/   .   .
 * 0   .   .   .   \___________________________________/   .   .   .
 *     .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
 *     .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
 *     .   .___________________    .   .   .   .   .   .   .   .   .
 *     .___/   .   .   .       \___    .   .   .   .   .   .   .   .
 * 1 __/   .   .   .   .   .       \________________________________
 *     .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
 *     .   .   .   .   .    ___________________    .   .   .   .   .
 *     .   .   .   .    ___/   .   .       .   \___.   .   .   .   .
 * 2 __________________/   .   .   .   .       .   \________________
 *     .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
 *     .   .   .   .   .   .   .   .   .    ___________________    .
 *     .   .   .   .   .   .   .   .   .___/       .   .   .   \___ 
 * 3 __________________________________/   .   .       .   .   .   \
 *
 * 
 */
/* 
 * Pinout for the PORTA as well as PORTC port
 */
#define H0	0x01	// half current, coil 0 - 1 0000 0001
#define F0	0x02    // full current, coil 0 - 2 0000 0010
#define H1	0x04    // half current, coil 1 - 4 0000 0100
#define F1	0x08    // full current, coil 1 - 8 0000 1000
#define H2	0x10	// half current, coil 2 - 16 0001 0000
#define F2	0x20    // full current, coil 2 - 32 0010 0000
#define H3	0x40    // half current, coil 3 - 64 0100 0000
#define F3	0x80    // full current, coil 3 - 128 1000 0000

static uint8_t phase[] = 
{
    F0, F0|H1, F0|F1, H0|F1,
    F1, F1|H2, F1|F2, H1|F2,
    F2, F2|H3, F2|F3, H2|F3,
    F3, F3|H0, F3|F0, H3|F0,
};


/* 
 * current position.
 * 
 * Initialize at left (away from home switch), and with the mat touching
 * the rollers, such that loading the mat is a simple movement to (0,0)
 */

volatile int x = -MAT_EDGE, y = 4800; 
volatile int phase_x = 0, phase_y = 0;
/*
 * current pressure
 */

static int pressure = MAX_PEN_PRESSURE;

/*
 * experimental soft pen drop parameters
 */
int pen_time[4] = { 10, 12, 40 };
static int pen_seq;

static struct bresenham
{
    int step;			// current step
    int steps;			// number of steps in main direction
    int delta;			// number of steps in other direction 
    int error;			// residual error
    int dx;			// x step direction
    int dy;			// y step direction
    char steep;			// y > x
} b;

static int delay;		// delay between steps (if not 0)

static enum state
{
    HOME1,			// homing until switch is pushed
    HOME2,			// reversing until switch is released
    READY,			// motor off, pen up
    LINE			// draw straight line
} state;

/*
 * command queue. The stepper controller takes commands from the queue
 * using step timer interrupt. Main program can put new commands in.
 * 
 * The idea behind this queue is to keep the movement engine as busy as
 * possible, by smoothly joining the end of one stroke with the beginning of
 * the next one, so ideally the whole path can be traced in one continuous
 * motion.
 */
	
#define CMD_QUEUE_SIZE 16	// must be power of two

enum type
{
    MOVE,			// move with pen up 
    DRAW,			// move with pen down
    SPEED,			// set speed
    PRESSURE,			// set pressure 
};

struct cmd
{
    enum type type;		// command type,
    int x, y; 			// target coordinates
} cmd_queue[CMD_QUEUE_SIZE];

static volatile uint8_t cmd_head, cmd_tail;

int stepper_queued( void )
{
    return (char) (cmd_head - cmd_tail);
}

void stepper_off( void )
{
    PORTA = 0;
    PORTC = 0;
}

/*
 * allocate a new command, fill in the type, but don't put it in the queue yet.
 */
static struct cmd *alloc_cmd( uint8_t type )
{
    struct cmd *cmd;

    while( (uint8_t) (cmd_head - cmd_tail) >= CMD_QUEUE_SIZE )
	wdt_reset( );
    cmd = &cmd_queue[cmd_head % CMD_QUEUE_SIZE];
    cmd->type = type;
    return cmd;
}

/*
 * get next command from the queue (called in ISR)
 */
static struct cmd *get_cmd( void )
{
    if( cmd_head == cmd_tail )
        return 0;
    else
        return &cmd_queue[cmd_tail++ % CMD_QUEUE_SIZE];
}

/*
 * move to coordinate (x, y) with pen lifted. We allow moving
 * beyond the mat so that it will roll out.
 */
void stepper_move( int x, int y )
{
    struct cmd *cmd = alloc_cmd( MOVE );

    if( x < -MAT_EDGE || x > MAX_X || y < 0 || y > MAX_Y )
        return;
    cmd->x = x;
    cmd->y = y;
    cmd_head++;
}

/************************************************************************
 Experimental code to move the motors when the user presses the arrow
 keys on the keypad.  Problem currently is that it will not move beyond
 0 on either axis. The x axis however is the mat and we should be able
 to move as far as we want to.  So, inside the stepper code we 
 will probably have to override the limits when we're moving manually
************************************************************************/
void stepper_move_manual(int direction)
{
	int move_amount=50;
	// dont move if we're already moving...
	if( cmd_head != cmd_tail ) {
		return;
	}
	int cur_x,cur_y;
	// move keys
	stepper_get_pos( &cur_x,&cur_y);
	switch(direction) {
		case KEYPAD_MOVEUP:
			cur_x+=move_amount;
			break;
		case KEYPAD_MOVEUPLEFT:
			cur_x+=move_amount;
			cur_y+=move_amount;
			break;
		case KEYPAD_MOVELEFT:
			cur_y+=move_amount;
			break;
		case KEYPAD_MOVEDNLEFT:
			cur_x-=move_amount;
			cur_y+=move_amount;
			break;
		case KEYPAD_MOVEDN:
			cur_x-=move_amount;
			break;
		case KEYPAD_MOVEDNRIGHT:
			cur_x-=move_amount;
			cur_y-=move_amount;
			break;
		case KEYPAD_MOVERIGHT:
			cur_y-=move_amount;
			break;
		case KEYPAD_MOVEUPRIGHT:
			cur_x+=move_amount;
			cur_y-=move_amount;
			break;
		
	}
	stepper_move(cur_x,cur_y);
	// what we need to implement is a way
	// to re-zero after a manual move.
	// this allows the user to manually move
	// the pen holder into position and 
	// start cutting from that position.
	// HPGL commands from inkscape always
	// start with 0,0 as the home position,

	
}
/*
 * draw to coordinate (x, y) with pen dropped. 
 */
void stepper_draw( int x, int y )
{
    struct cmd *cmd = alloc_cmd( DRAW );

    if( x < 0 || x > MAX_X || y < 0 || y > MAX_Y )
        return;
    cmd->x = x;
    cmd->y = y;
    cmd_head++;
}


void stepper_speed( int speed )
{
    struct cmd *cmd = alloc_cmd( SPEED );
    cmd->x = speed;
    cmd_head++;
}

/*
 * loading paper. If x < 0, the mat needs to be pulled under the rollers first.
 * This is a heavy operation, so it's done at reduced speed. In all other
 * cases, just move the pen to the origin.
 */
void stepper_load_paper( void )
{
    if( x < 0 )
    {
		//stepper_speed( 8 ); 	
		stepper_move( 0, y );	// don't move Y direction yet (it may be slow)
    }
    //stepper_speed( 8 );
    stepper_move( 0, 0 );
}

/* 
 * unloading the paper is simple, just go to a position outside the drawing
 * area, and the mat will roll out.
 */
void stepper_unload_paper( void )
{
    //stepper_speed( 8 );
    stepper_move( -MAT_EDGE, 0 );
}

/*
 * set the pressure on the pen.
 */
void stepper_pressure( int pressure )
{
    struct cmd *cmd = alloc_cmd( PRESSURE );
    cmd->x = pressure;
    cmd_head++;
}

void stepper_set_pen_pressure_override(int p)
{
	pressure = p;
}

/* 
 * get current position. Only an indication for debugging purposes. 
 * During movement, this position changes asynchronously.
 */
void stepper_get_pos( int *px, int *py )
{
    *px = x;
    *py = y;
}

/*
 * The original firmware also removes the PWM signal, but it seems 
 * to work ok when you leave it on.
 */
void pen_up( void )
{
    if( PORTE & PEN )
        delay = 50;
    PORTE &= ~PEN;
    pen_seq = -1;
}

/*
 * move pen down
 */
 void pen_down( void )
{
	int pe=PORTE;
	
    if( pe & PEN )	// already down, ignore
       return;
    /*
     * soft pen drop with lowest pressure
     */
    //timer_set_pen_pressure( 1023 );
    PORTE |= PEN;		
    pen_seq = 0;
    delay = pen_time[2];
}

/*
 * initialize Bresenham line drawing algorithm. Draw from (x, y) 
 * to (x1, y1).
 */
static void bresenham_init( int x1, int y1 )
{
    int dx, dy;

    if( x1 > x ) 
    { 
	b.dx = 1;
        dx = x1 - x;
    }
    else
    {
	b.dx = -1;
	dx = x - x1;
    }
    if( y1 > y )
    {
	b.dy = 1;
        dy = y1 - y;
    }
    else
    {
	b.dy = -1;
        dy = y - y1;
    }
    if( dx > dy )
    {
	b.steep = 0;
        b.steps = dx;
	b.delta = dy;
    }
    else
    {
	b.steep = 1;
        b.steps = dy;
	b.delta = dx;
    }
    b.step = 0;
    b.error = b.steps / 2;
}

/*
 * perform single step in Bresenham line drawing algorithm.
 */
static void bresenham_step( void )
{
    if( b.step >= b.steps )
    {
        state = READY;
        return;
    }
    b.step++;
    if( (b.error -= b.delta) < 0 )
    {
        b.error += b.steps;
	x += b.dx;
	y += b.dy;
    }
    else if( b.steep )
	y += b.dy;
    else
	x += b.dx;
}

/*
 * get next command from command queue.
 */
enum state do_next_command( void )
{
    struct cmd *cmd = get_cmd( );

    if( !cmd )
    { 
       //pen_up( );
       return READY;
    }
    switch( cmd->type )
    {
	case MOVE: 
	case DRAW: 
	    // FIXME: avoid null-movements earlier 
	    if( x == cmd->x && y == cmd->y )
	        return READY;
	    if( cmd->type == MOVE )
			pen_up(); 
	    else
	        pen_down( );
	    bresenham_init( cmd->x, cmd->y );
	    return LINE;
	case PRESSURE:
	    pressure = cmd->x;
	    timer_set_pen_pressure( pressure );
	    break;

	case SPEED:
	    timer_set_stepper_speed( cmd->x );
	    break;
    }
    return READY;
}

/*
 * This function is called by a timer interrupt. It does one motor step 
 * (if active).
 */
void stepper_tick( void )
{
    /*
     * experimental soft pen drop.
     * A. At time=0, pen is dropped with minimal pressure
     * B. At time=pen_time[0] pen is lifted again
     * C. At time=pen_time[1] pen is dropped with set pressure
     *
     * The B-C interval should be short enough such that the pen 
     * isn't actually going back up, but just loses most of its
     * downward speed. This fixed the 'heavy dot' problem at the 
     * the start of a stroke. Unfortunately, there's still a 
     * 'heavy dot' when the pen is lifted. 
     *
     * Timing is still tied to stepper tick units, which should be 
     * fixed. 
     */
	/*
    if( pen_seq >= 0 )
    {
	pen_seq++;
	if( pen_seq >= pen_time[1] )
	{
	    PORTE |= PEN;
	    timer_set_pen_pressure( pressure );
	    pen_seq = -1;
	}
	else if( pen_seq >= pen_time[0] )
	    PORTE &= ~PEN;
    }
	
    */
    if( delay && --delay )
        return;
	
    // abort cutting if 'STOP' is pressed.
    if( keypad_stop_pressed() )	
    {
        state = READY;
		x = 0;
		cmd_tail = cmd_head;
		pen_up();
		stepper_off( );
    }
		
again:
    switch( state )
    {
	case HOME1:
	    if( !at_home() && y > 0 )
	    	y--;
	    else
		state = HOME2; 
	    break;

    	case HOME2:
	    if( at_home() )
	       y++;
	    else
	    {
	        y = 0;
		state = READY;
	    }
	    break;

	case READY:
	    state = do_next_command( );
	    if( state != READY )
	        goto again;
	    break;
     
     	case LINE:
		    bresenham_step( );
	    break;
    }
    if( state == READY )
    {
	/* 
	 * the motors get quite hot when powered on, so we turn them off
	 * when idling. Ideally, you'd want to leave them on, so the user
	 * can't move the pen/mat around. Maybe leave them at half-power ?
	 */
        stepper_off( );		
    }
    else
    {
		PORTA = phase[x & 0x0f];	// low 4 bits determine phase
		PORTC = phase[y & 0x0f];
    }
}

void stepper_init( void )
{
    stepper_off( );
    pen_up( );
    //DDRB |= DEBUG;
    DDRA = 0xff;
    DDRC = 0xff;
    DDRE |= PEN;
    PORTD |= HOME; // enable pull-up
}
