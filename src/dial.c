/*
 * dial.c
 *
 * Driver for 3 analog dials, connected as follows:
 *
 * dial  | AVR
 *-------+-----------
 * size  | PF0 (ADC0)
 * speed | PF1 (ADC1)
 * press | PF2 (ADC2)
 *
 * Each input is a simple voltage divider between 0 and 5V, with 
 * a few discrete settings where the pot clicks.
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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include "dial.h"
#include "timer.h"
#include "display.h"

/*
* The dials are not working on my current machine.  Suspect I fried the 
* ADC port or something.  The thing is, there are two dial connections
* on the motherboard, one at the top and one at the bottom
* They both seem to go up to the ADC pins on the ATMEGA1281
* but so far I have not been able to find a faulty part.
* I did find some faulty parts, but not related..  two burned resistors.
* How did they get burned.....One was even shorted.
*/


static uint8_t channel = 0;
static unsigned char pvars[4]; // holds previous values

static unsigned char dial_adc[MAX_DIALS];
static unsigned char dial_steps[MAX_DIALS] = { 25, 5, 5 };

static int dial_setting( uint8_t dial )
{
    return( (dial_adc[dial] * (dial_steps[dial]-1) + 125) / 250 );
}

int dial_get_speed( void )
{
    return dial_setting( DIAL_SPEED );
}

int dial_get_pressure( void )
{
    return dial_setting( DIAL_PRESSURE );
}

int dial_get_size( void )
{
    return dial_setting( DIAL_SIZE );
}

void dial_poll( void )
{
    ADCSRA |= (1 << ADIF);
    dial_adc[channel] = ADCH;
    if( ++channel == MAX_DIALS )
        channel = 0;
    ADMUX = (1 << ADLAR) | (1 << REFS0) | channel;
    ADCSRA |= (1 << ADSC);	
	char string[40];
	unsigned char i=dial_get_speed();			
	if(pvars[0]!=i) {
		sprintf(string,"Dial speed: %d\r",i);
		display_puts( string );
		beep();
		pvars[0]=i;
	}

	i=dial_get_size();
	if(pvars[1]!=i) {
		sprintf(string,"Dial size: %d\r",i);
		display_puts( string );
		beep();
		pvars[1]=i;
	}

	i=dial_get_pressure();
	if(pvars[2]!=i) {
		// need to come up with a sexier way
		// when the user turns the wheel, the 
		// pen's pressure should go from
		// softest to hardest
		// four increments in between
		// would actually like to swap this for the
		// speed wheel that is right side
		// which is a free turning wheel
		// with 25 increments.  use it to select
		// Material.  Maybe allow the user to 
		// program their own materials with
		// speed/pressure/multi-cut settings
		timer_set_pen_pressure(i*50);
		sprintf(string,"Dial pressure: %d\r",i);
		display_puts( string );
		beep();
		pvars[2]=i;
	}
	
}

void dial_init( void )
{
	/*
	REFS1 REFS0 Voltage Reference Selection(1)
	0 0 AREF, Internal VREF turned off
	0 1 AVCC with external capacitor at AREF pin
	*/
    ADMUX = (1 << ADLAR) | ( 1 << REFS0) | channel;
    //ADMUX = (1 << ADLAR) | channel;
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2) | (1<<ADPS1) |(1<<ADPS0); // prescaler 128
}
