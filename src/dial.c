/*
 * dial.c
 *
 * Driver for 3 analog dials, connected as follows:
 *
 * dial  | AVR
 *-------+-----------
 * size  | PF0 (ADC0   ------------   On some machines this is a quadrature encoder, not a pot
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
#include "keypad.h"
#include "dial.h"
#include "timer.h"
#include "display.h"

static uint8_t channel = 0;
static unsigned char pvars[MAX_DIALS]; // holds previous values

static unsigned char dial_adc[MAX_DIALS];
static unsigned char dial_steps[MAX_DIALS] = { 25, 5, 5 }; // weak association !! should use struct instead 

static int dial_setting( uint8_t dial )
{	
	int step =(dial_adc[dial] * (dial_steps[dial]-1) + 125) / 250;
	
    return step+3;		 // +3 to have the dials set speed and pressure in the range of 3-7 -- middle of overall range 0-9
}

void dial_poll( void )
{
	unsigned char i;
	static unsigned char  queued = 0;
	char string[40];
    
	// records ADC readings from 2 or 3 ADC inputs on a round robin basis 	
    dial_adc[channel] = ADCH;
    
	if( ++channel == MAX_DIALS )
	{
        channel = 0;
		queued = 1;
	}
    
	// setup for next measurement, 
	ADMUX = (1 << ADLAR) | (1 << REFS0) | channel;	// Change input channel
    ADCSRA |= (1 << ADSC);							// start next conversion so it's ready to be read next time we come around 
	
	if (! queued )		// wait until all ADC channels are read at least once before looking  at the  readings
		return;
		
	if(pvars[DIAL_SPEED] != (i=dial_setting( DIAL_SPEED))) 
	{
		sprintf(string,"Dial speed: %d\r",i);
		display_puts( string );
		beep();
		pvars[DIAL_SPEED]=i;
		timer_set_stepper_speed(i);		
		keypadSet_Speed_state( );	// so that the +/- keys follow the speed settings
		
	}
	
	if(pvars[DIAL_PRESSURE] != (i=dial_setting( DIAL_PRESSURE )))
	{
		sprintf(string,"Dial pressure: %d\r",i);
		display_puts( string );
		beep();
		pvars[DIAL_PRESSURE]=i;
		timer_set_pen_pressure(i);
		keypadSet_Pressure_state( );	// so that the +/- keys follow the Pressure settings
	}
		
#ifdef SIZE_WHEEL_IS_POTENTIOMETER
	if(pvars[DIAL_SIZE] != (i=dial_setting( DIAL_SIZE ))) 
	{
		sprintf(string,"Dial size: %d\r",i);
		display_puts( string );
		beep();
		pvars[DIAL_SIZE]=i;
	}
#endif
	
	

}

void dial_init( void )
{
	/*
	REFS1 REFS0 Voltage Reference Selection(1)
	0 0 AREF, Internal VREF turned off
	0 1 AVCC with external capacitor at AREF pin
	*/
	// DIDR0 = 0x7;  // disable digital input buffers on ADC0 to ADC2 --- mostly for current consumption though, potentiometers are low in impedance so it doesn't matter  
    ADMUX = (1 << ADLAR) | ( 1 << REFS0| channel ) ;  // 8 bit read mode, left justified result -- read only ADCH, using VCC for ADCref
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2) | (1<<ADPS1) |(1<<ADPS0); // Enable ADC, set ADC clock pre-scaler to 128 and start a conversion
}
