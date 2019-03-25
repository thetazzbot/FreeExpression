/*
 * keypad.h
 *
 * Created: 3/23/2019 11:02:59 PM
 *  Author: adeva
 */ 


#ifndef KEYPAD_H_
#define KEYPAD_H_

#ifdef MACHINE_CAKE
	#include "keypad_cake.h"
#endif
#ifdef MACHINE_EXPRESSION
	#include "keypad_expression.h"
#endif

int keypad_poll( void );
void keypad_init( void );
void keypad_set_leds( uint16_t mask );
char keypad_stop_pressed( void );
void keypadSet_Speed_state( void );
void keypadSet_Pressure_state( void );
int keypad_scan( void );
#define leds_on()	do { PORTD &= ~(1 << 5); } while(0)
#define leds_off()	do { PORTD |=  (1 << 5); } while(0)


typedef enum _languge {
	HPGL=1,
	G_CODE,
	GPGL
} en_language;

extern en_language  Lang;




#endif /* KEYPAD_H_ */