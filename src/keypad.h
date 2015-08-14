/* keypad.h
 *
 		This source original developped by  https://github.com/Arlet/Freecut
 
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
 */

#ifndef _KEYPAD_H
#define _KEYPAD_H

/* 
 * These two defines indicate the X-Y size of the keyboard matrix and has to match the actual Hardware. 
 * of the Expression and cake machine.
 * Do not change arbitrarily, but if your machine has a different size matrix make sure you learn the 
 * key-codes that go together with a different size matrix first.
 */ 

#define KBD_MAX_COLS	20
#define KBD_MAX_ROWS 	5
enum {
  
	KEYPAD_F1 =0,
	KEYPAD_F2,
	KEYPAD_1,
	KEYPAD_2,
	KEYPAD_3,
	KEYPAD_4,
	KEYPAD_5,
	KEYPAD_6,
	KEYPAD_7,
	KEYPAD_8,
	KEYPAD_9,
	KEYPAD_0, 
	KEYPAD_SPACE,  
	KEYPAD_BACKSPACE,   
	KEYPAD_PORTRAIT, 
	KEYPAD_FLIP, 
	KEYPAD_MINUS,  
	KEYPAD_OK, 
	KEYPAD_PLUS, 
	KEYPAD_XTRA1, 
	KEYPAD_F3,  
	KEYPAD_F4,  
	KEYPAD_Q,
	KEYPAD_W,   
	KEYPAD_E, 
	KEYPAD_R,
	KEYPAD_T,
	KEYPAD_Y, 
	KEYPAD_U, 
	KEYPAD_I, 
	KEYPAD_O, 
	KEYPAD_P,  
	KEYPAD_ClearDisplay,
	KEYPAD_RESETALL, 
 	KEYPAD_FITPAGE,
	KEYPAD_QUANTITY=35,
	// some missing buttons that are not on the machine
	KEYPAD_XTRA2=39,
	KEYPAD_F5, 
	KEYPAD_F6,
	KEYPAD_A,   
	KEYPAD_S,    
	KEYPAD_D,   
	KEYPAD_F,   
	KEYPAD_G,   
	KEYPAD_H,  
	KEYPAD_J,  
	KEYPAD_K,   
	KEYPAD_L,
	KEYPAD_AMPERSAND,
	KEYPAD_REPEATLAST,
	KEYPAD_SOUNDONOFF,
	KEYPAD_MIXMATCH,
	KEYPAD_MULTICUT,	
	KEYPAD_MOVEUPLEFT,
	KEYPAD_MOVEUP,
	KEYPAD_MOVEUPRIGHT,
	KEYPAD_LINERETURN, 
	KEYPAD_MATERIALSAVER,
	KEYPAD_REALDIALSIZE,
	KEYPAD_Z, 
	KEYPAD_X,   
	KEYPAD_C,   
	KEYPAD_V,   
	KEYPAD_B,   
	KEYPAD_N,   
	KEYPAD_M,
	KEYPAD_COMMA,
	KEYPAD_PERIOD, 
	KEYPAD_SLASH,   
	KEYPAD_LOADLAST, 
	KEYPAD_SETPAPERSIZE,
	KEYPAD_FITLENGTH,
	KEYPAD_CENTERPOINT,
	KEYPAD_MOVELEFT,
	KEYPAD_CUT, 
	KEYPAD_MOVERIGHT,
	KEYPAD_SETTINGS, 
	KEYPAD_SHIFT, 
	KEYPAD_CUT_SHIFTLOCK,
	KEYPAD_EQUALS,   
	KEYPAD_LEFTBRACKET, 
	KEYPAD_RIGHTBRACKET, 
	KEYPAD_LEFTBRACE,   
	KEYPAD_RIGHTBRACE,   
	KEYPAD_QUOTE,   
	KEYPAD_49, 
	KEYPAD_50=90,  
	// Missing button 
	KEYPAD_LOADMAT=92, 
	KEYPAD_UNLOADMAT, 
	KEYPAD_AUTOFILL, 
	KEYPAD_MATSIZE, 
	KEYPAD_MOVEDNLEFT, 
	KEYPAD_MOVEDN , 
	KEYPAD_MOVEDNRIGHT 
} KEYS;

int keypad_poll( void );
void keypad_init( void );
void keypad_set_leds( uint16_t mask );
char keypad_stop_pressed( void );
extern void keypadSet_Pressure_state( void);
extern void keypadSet_Speed_state( void);
#define leds_on()	do { PORTD &= ~(1 << 5); } while(0)
#define leds_off()	do { PORTD |=  (1 << 5); } while(0)

typedef enum _languge {
	HPGL=1,
	G_CODE,
	GPGL
	} en_language;
	
extern en_language  Lang;


#endif
