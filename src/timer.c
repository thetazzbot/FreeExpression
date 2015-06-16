/*
 * timer.c
 *
 * Timer 0 is used for stepper timing
 * Timer 1 is used as solenoid PWM, through OC1B output
 * Timer 2 is used as overall (slow) event timer, as well sleep delay timer.
 * Timer 3 is used to generate tones on the speaker through OC3A output, 
 * period is adjusted for tone.
 *
 * This file is part of FreeExpression.
 *
 * https://github.com/thetazzbot/FreeExpression
 *
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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdio.h>
#include "timer.h"
#include "stepper.h"
#include "display.h"
static uint8_t count_Hz = 250;
static uint8_t count_25Hz = 10;
volatile uint8_t flag_Hz;
volatile uint8_t flag_25Hz;
static int current_pen_pressure; 
static int current_stepper_speed; 



/*
 * called @250 Hz, divide further in software for slow events 
	TCCR2A = (1 << WGM21);     // CTC
	TCCR2B = (1 << CS21) | (1 << CS20) ; //timer2's prescaler is different than the rest... set two bits instead of one
	OCR2A  = 249;  // value to count to, CTC interrupts when this value is met
	TIMSK2 = (1 << OCIE2A); // enable interrupt
 */
ISR( TIMER2_COMPA_vect ) 
{
    if( --count_25Hz == 0 )
    {
        count_25Hz = 10;
        flag_25Hz = 1;
    }
    if( --count_Hz == 0 )
    {
		count_Hz = 250;
		flag_Hz = 1;
    }
	
}        

/*
 * Timer 0 compare match, update stepper motors.
	TCCR0A = (1 << WGM01);     // CTC
	TCCR0B = (1 << CS00) ;     // prescaler 1/256 -> 250 kHz
	OCR0A  = 99;  // value to count to, CTC interrupts when this value is met
	TIMSK0 = (1 << OCIE0A); // enable interrupt
 */
ISR( TIMER0_COMPA_vect ) 
{
    stepper_tick( );
}        

/*
 * Turn on beeper. Hz specifies frequency of the tone.
 */
void beeper_on( int Hz )
{
    DDRE |= (1 << DDE3);
    OCR3A = (F_CPU + Hz/2) / Hz - 1;
}

void beeper_off( void )
{
    DDRE &= ~(1 << DDE3);
}

/*
 * usleep: sleep (approximate/minimum) number of microseconds. We use timer2 
 * which runs at 62.50 kHz, or at 16 usec/tick. Maximum delay is about 2 
 * milliseconds . For longer delays, use msleep().
 *
 */
void usleep( int usecs )
{
    signed char end = TCNT2 + usecs / 16;

    while( (signed char) (TCNT2 - end) < 0 )
	continue;
}

void msleep( unsigned msecs )
{
    while( msecs-- != 0 )
	usleep( 1000 );
}

int timer_get_stepper_speed()
{
	return current_stepper_speed;
//	return OCR0A+1;
}

void beep()
{
	beeper_on( 1760 );
	msleep( 10 );
	beeper_off( );
	
}
void timer_set_stepper_speed( int delay )
{
	// delay is displayed in single increment steps
	// but internally each step represents 25 usecs delay in timer
// testing limits from 0 to 255
	current_stepper_speed = delay;
	char string[40];
	sprintf(string,"Speed: %d",current_stepper_speed);
	display_puts(string);
	
	delay=(255 - (delay*25)); 
	if(delay>255) delay=255;
	if(delay<1) delay=1;
    uint8_t prescaler = 4; // default 1:64 prescaler
    TCCR0B &= ~7;  // stop timer, and clear prescaler bit
    if( delay > 256 )
    {
        delay /= 4;
	    prescaler = 5;
    }
    OCR0A = delay - 1;
    TCCR0B |= prescaler;
}

int timer_get_pen_pressure()
{
	return current_pen_pressure;
}
/**
* Sets the pen pressure according to a value from MIN_PEN_PRESSURE to MAX_PEN_PRESSURE.
* 
* @param pressure Integer 
*/

void timer_set_pen_pressure( int pressure )
{
	// pen pressure is displayed in single increment steps
	// but internally each step represents 50 usecs delay in pwm
	// max_pen_pressure is the max value, not the maxium pressure
	// maxium value of 500 is basically no pressure at all
	// it could go as high as 1000 but that is pointless
	// since anything between 500 and 1000 results in 6 volts
	// which the solenoid doesnt care about.
    if( pressure > MAX_PEN_PRESSURE/50 )
        pressure = MAX_PEN_PRESSURE/50;
	if(pressure < 1) pressure=1;
	current_pen_pressure = pressure;
	char string[40];
	sprintf(string,"Pressure: %d",pressure);
	display_puts(string);

    OCR1B  = (MAX_PEN_PRESSURE- (pressure*50));	
}

/*
 * Init timers 
 */
void timer_init( void )
{
	//ATMega1281 - Used in Cricut Expression CREX001
	// set timer 0, variable period for stepper

	TCCR0A = (1 << WGM01);     // CTC
	TCCR0B = (1 << CS00) | (1<<CS01) ;     // prescaler 1/256 -> 250 kHz << this doesnt compute @16mhz/256=62500
	OCR0A  = 99;  // value to count to, CTC interrupts when this value is met
	TIMSK0 = (1 << OCIE0A); // enable interrupt

	// set timer 2 for 250 Hz period
	
	TCCR2A = (1 << WGM21);     // CTC
	TCCR2B = (1 << CS21) | (1 << CS20) ;       //timer2's prescaler is different than the rest... set two bits instead of one
	OCR2A  = 249;  // value to count to, CTC interrupts when this value is met
	TIMSK2 = (1 << OCIE2A); // enable interrupt
	
	
    DDRB   |= (1 << PB6);				// PB6 is PWM output, oc1b
    // set timer 1, WGM mode 7, fast PWM 10 bit
    // PWM, Phase Correct, 10-bit
	//Clear OCnA/OCnB/OCnC on compare match, set OCnA/OCnB/OCnC at BOTTOM (non-inverting mode)
    TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
    OCR1B  = 1023;				
    TCCR1B = (1 << WGM12) | (1 << CS10);	// 00001001 wave form generation mode,CLKio no prescaling


    // Timer 3, WGM mode 15 (1111), Fast PWM using OCR3A 
	// this is used by the beeper, OCR3A is set in beeper_on(hz)
    TCCR3A = (1 << COM3A0) | (1 << WGM31) | (1 << WGM30);
    TCCR3B = (1 << WGM33) | (1 << WGM32) | 1;   


	
}
