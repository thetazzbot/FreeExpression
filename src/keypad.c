/*
 * keypad.c
 *
 * Keypad is designed as a matrix with 24 cols, and 5 c. The 15 rows
 * are selected using a 16 bit shift register, with CLK/Data inputs.
 * 
 * NOTE: Due to this design, if you press two (or more) keys in the same row
 * simultaneously, the the two corresponding outputs of the shift register
 * are shorted through the keys, and the CPU gets undefined results.
 * 
 * Pinout of the connector is as follows:
 *
 * Pin 	|  Name	| AVR  | Description
 *------+-------+------+------------
 *  1   |  GND 	|      |
 *  2   |  Vcc  |      |
 *  3   |  Stop | PD0  | Direct connection to stop button
 *  4   |  O4   | PG4  | Row output 2 (bottom)
 *  5   |  O3   | PG3  | Row output 4
 *  6   |  O2 	| PG2  | Row output 3
 *  7   |  O0   | PG0  | Row output 0 (top row)
 *  8   |  LED 	| PD5  | LED Enable (0=LEDs on)
 *  9   |  O1   | PG1  | Row output 1
 * 10   | CLK   | PD7  | Shift register clock
 * 11   | Data  | PD6  | Shift register data in
 * 12   | Test  |      | Shift register data out (not used)
 * 13	| NC	| NC   |
 * 14	| NC	| NC   | 	
 * Key layout is straightforward: column 0 is left, column 13 is right, 
 * row 0 is top, row 4 is bottom. The grey arrow keys and CUT key are 
 * mapped in column 14:
 * 
 * Row  | Key
 *------+------
 *  0   | Right
 *  1   | Left 
 *  2   | CUT 
 *  3   | Up 
 *  4   | Down 
 * 
 * The various LEDs are also connected to the shift register outputs, 
 * and can be turned on with the LED Enable pin (0=On, 1=Off).  
 *
 * LED Layout is as follows:
 *
 * COL0  COL1
 * COL2  COL3
 * COL4  COL5				COL10 (LED between CUT and Up arrow)
 * COL6  COL7
 * COL8  COL9
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdio.h>
#include "keypad.h"
#include "timer.h"
#include "stepper.h"
#include "display.h"
#include "flash.h"
// pin assignment
#define STOP	(1 << 0)	// PD0
#define LEDS	(1 << 5)	// PD5
#define DATA	(1 << 6) 	// PD6
#define CLK	(1 << 7)	// PD7
#define ROWS    (0x1f)		// mask for PG4-0

#define MAX_COLS	24	
#define MAX_ROWS 	5	


#define clk_h()		do { PORTD |=  CLK;  } while(0)
#define clk_l()		do { PORTD &= ~CLK;  } while(0)
#define data_h()	do { PORTD |=  DATA; } while(0)
#define data_l()	do { PORTD &= ~DATA; } while(0)
#define get_rows()	(~PING & ROWS)

uint8_t keypad_state[MAX_COLS];	// current state
uint8_t keypad_prev[MAX_COLS];  // previous state
uint16_t leds;

static int k_state=0;
/*
 * keypad_write_cols: write all 16 column bits in 'val' to shift register.
 */
static void keypad_write_cols( short val )
{
    int i;

    for( i = 0; i < MAX_COLS; i++ )
    {
        if( val < 0 ) 
	    data_h( );
	else
	    data_l( );
	clk_h( );
	val <<= 1;
	clk_l( );
    }
}

void keypad_set_leds( uint16_t mask )
{
    leds = mask;
    leds_off( );
    keypad_write_cols( ~leds );
    leds_on( );
}

/*
 * return state of 'STOP' key.
 */
char keypad_stop_pressed( void )
{
	int c=(PIND & STOP);
    return !c;
}

/* 
 * keypad_scan: perform a single scan of keyboard. Returns keycode of 
 * key that was pressed (or -1 if nothing).  
 */
int keypad_scan( void )
{
    int row, col;
    int pressed = -1;

    leds_off( );		// turn off LEDs during scan
    keypad_write_cols( ~1 );	// single zero at column 0
    data_h( );			// shift in ones
    for( col = 0; col < MAX_COLS; col++ )
    {
			keypad_state[col] = get_rows( );
			clk_h( );
			clk_l( );
    }
   // keypad_write_cols( ~leds );
   // leds_on( );	

    // keyboard has been scanned, now look for pressed keys
    for( col = 0; col < MAX_COLS; col++ )
    {
		uint8_t diff = keypad_state[col] ^ keypad_prev[col];

		if( diff )
		{
			for( row = 0; row < MAX_ROWS; row++ )
			{
				uint8_t mask = 1 << row;
				if( diff & mask & keypad_state[col] )
				pressed = row * 24 + col;
			}
		}
		keypad_prev[col] = keypad_state[col];
    }
    return pressed;
}


//#define LOAD_PAPER	0x4c
//#define UNLOAD_PAPER	0x4d

void 
keypadSet_Speed_state( void )
{
	k_state = KEYPAD_XTRA2;
	
}
void 
keypadSet_Pressure_state( void )
{
	k_state = KEYPAD_XTRA1;
	
}
//keypad_poll is here instead of in keypad.c due to calling of various actions like load paper, etc.
int keypad_poll( void )
{
	int key = keypad_scan( );
	char string[40];
	switch( key )
	{
		case KEYPAD_LOADMAT:
		stepper_load_paper();
		break;

		case KEYPAD_UNLOADMAT:
		stepper_unload_paper();
		break;
		
		// move keys
		// don't move if we're already moving...
		case KEYPAD_MOVEUP:
		case KEYPAD_MOVEUPLEFT:
		case KEYPAD_MOVELEFT:
		case KEYPAD_MOVEDNLEFT:
		case KEYPAD_MOVEDN:
		case KEYPAD_MOVEDNRIGHT:
		case KEYPAD_MOVERIGHT:
		case KEYPAD_MOVEUPRIGHT:
		stepper_move_manual(key);
		break;
		
		
#ifdef DEBUG_FLASH		
		case KEYPAD_F1:
		flash_test();
		break;
#endif
		case KEYPAD_1:
		display_puts("Cutter down");
		pen_down();
		break;
		
		case KEYPAD_2:
		display_puts("Cutter up");
		pen_up();
		break;
		
		
		case KEYPAD_XTRA1:
		{
			int p=timer_get_pen_pressure();
			sprintf(string,"Pressure: %d",p);
			display_puts(string);
			k_state=key;
			break;
		}
		case KEYPAD_XTRA2: 
		{
			int p=timer_get_stepper_speed();
			sprintf(string,"Speed: %d",p);
			display_puts(string);
			k_state=key;
			break;
		}
		case KEYPAD_CUT:
			k_state=key;
			break;
			
		case KEYPAD_MINUS:
			if(k_state==KEYPAD_XTRA1) { // PRESSURE SET
				// pressure is inversely related to 1023, min pressure
				int p=timer_get_pen_pressure()-1;
				 timer_set_pen_pressure(p);
			}
			if(k_state==KEYPAD_XTRA2) {  // SPEED SET
				int p=timer_get_stepper_speed()-1;
				timer_set_stepper_speed(p);
			}
			break;
			
		case KEYPAD_PLUS:
			if(k_state==KEYPAD_XTRA1) { // PRESSURE SET
				// pressure is inversely related to 1023, min pressure
				int p=timer_get_pen_pressure()+1;
				timer_set_pen_pressure(p);
			}
			if(k_state==KEYPAD_XTRA2) { // SPEED SET
				int p=timer_get_stepper_speed()+1;
				timer_set_stepper_speed(p);
			}
		break;
		default:
			if (key > 0 )
			{
				beeper_on( 2400 );
				msleep( 30 );
				beeper_off( );
			}
			break;
			
		
	}

	return key;
}
void keypad_init( void )
{
    clk_l( );
    leds_off( );
    DDRD |= (LEDS | DATA | CLK); 
    PORTD |= STOP;
    DDRG &= ~ROWS;  // rows are inputs
    PORTG |= ROWS;  // enable internal pull-ups
}

