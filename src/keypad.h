/* keypad.h
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
 */

#ifndef _KEYPAD_H
#define _KEYPAD_H

enum {
	KEYPAD_F1,  /** 00 **/
	KEYPAD_F2,  /** 01 **/
	KEYPAD_1,   /** 02 **/
	KEYPAD_2,   /** 03 **/
	KEYPAD_3,   /** 04 **/
	KEYPAD_4,   /** 05 **/
	KEYPAD_5,   /** 06 **/
	KEYPAD_6,   /** 07 **/
	KEYPAD_7,   /** 08 **/
	KEYPAD_8,   /** 09 **/
	KEYPAD_9,   /** 10 **/
	KEYPAD_0,   /** 11 **/
	KEYPAD_SPACE,   /** 12 **/
	KEYPAD_BACKSPACE,   /** 13 **/
	KEYPAD_PORTRAIT, /** 14 **/
	KEYPAD_FLIP, /** 15 **/
	KEYPAD_MINUSARROW = 16, /** 16 **/
	KEYPAD_OK, /** 17 **/
	KEYPAD_PLUSARROW, /** 18 **/
	KEYPAD_XTRA1, /** 19 **/
//	KEYPAD_STOP, /** **/
	KEYPAD_MINUS=16,   /** 16 **/
	KEYPAD_PLUS=18,   /** 18 **/
	KEYPAD_Q=18,   /** 18 **/
	KEYPAD_W,   /** 19 **/
	KEYPAD_E,   /** 20 **/
	KEYPAD_R,   /** 21 **/
	KEYPAD_T,   /** 22 **/
	KEYPAD_Y,   /** 23 **/
	KEYPAD_U,   /** 24 **/
	KEYPAD_F3 = 24,  /** 24 **/
	KEYPAD_F4=25,  /** 25 **/
	KEYPAD_I,   /** 25 **/
	KEYPAD_O,   /** 26 **/
	KEYPAD_P,   /** 27 **/
	KEYPAD_CHARDISPLAY,   /** 28 **/
	KEYPAD_RESETALL,   /** 29 **/
	KEYPAD_FITPAGE, /** 30 **/
	KEYPAD_QUANTITY,/** 31 **/
	KEYPAD_F5,   /** 32 **/
	KEYPAD_F6,   /** 33 **/
	KEYPAD_A,   /** 34 **/
	KEYPAD_S = 35,   /** 35 **/
	KEYPAD_D,   /** 36 **/
	KEYPAD_F,   /** 37 **/
	KEYPAD_G,   /** 38 **/
	KEYPAD_H,   /** 39 **/
	KEYPAD_J,   /** 40 **/
	KEYPAD_K,   /** 41 **/
	KEYPAD_L,   /** 42 **/
	KEYPAD_SEMICOLON,  /** 43 **/
	KEYPAD_XTRA2=43, /** 43 **/
	KEYPAD_REPEATLAST,   /** 44 **/
	KEYPAD_SOUNDONOFF,   /** 45 **/
	KEYPAD_MIXMATCH, /** 46 **/
	KEYPAD_MULTICUT, /** 47 **/
	KEYPAD_MATERIALSAVER,   /** 48 **/
	KEYPAD_REALDIALSIZE,   /** 49 **/
	KEYPAD_Z,   /** 50 **/
	KEYPAD_X,   /** 51 **/
	KEYPAD_LINERETURN = 51, /** 51 **/
	KEYPAD_C,   /** 52 **/
	KEYPAD_V,   /** 53 **/
	KEYPAD_B,   /** 54 **/
	KEYPAD_N,   /** 55 **/
	KEYPAD_M,   /** 56 **/
	KEYPAD_COMMA,   /** 57 **/
	KEYPAD_PERIOD,   /** 58 **/
	KEYPAD_SLASH,   /** 59 **/
	KEYPAD_LOADLAST,   /** 60 **/
	KEYPAD_SETCUTAREA,   /** 61 **/
	KEYPAD_FITLENGTH, /** 62 **/
	KEYPAD_CENTERPOINT, /** 63 **/
	KEYPAD_SHIFT,   /** 64 **/
	KEYPAD_MOVEUPLEFT = 64,
	KEYPAD_CUT_SHIFTLOCK,   /** 65 **/
	KEYPAD_MOVEUP = 65,
	KEYPAD_MOVEUPRIGHT = 66,
	KEYPAD_SETTINGS = 67, /** 67 **/
	KEYPAD_EQUALS,   /** 68 **/
	KEYPAD_LEFTBRACKET,   /** 69 **/
	KEYPAD_RIGHTBRACKET,   /** 70 **/
	KEYPAD_LEFTBRACE,   /** 71 **/
	KEYPAD_RIGHTBRACE,   /** 72 **/
	KEYPAD_QUOTE,   /** 73 **/
	KEYPAD_49,   /** 74 **/
	KEYPAD_50,   /** 75 **/
	KEYPAD_CUT=89, /** 89 **/
	KEYPAD_LOADMAT=108,   /** 76 **/
	KEYPAD_UNLOADMAT=109,   /** 77 **/
	KEYPAD_AUTOFILL, /** 78 **/
	KEYPAD_MATSIZE, /** 79 **/
	KEYPAD_MOVELEFT = 88,
	KEYPAD_MOVERIGHT = 90,
	KEYPAD_MOVEDNLEFT = 112, /** 80 **/
	KEYPAD_MOVEDN = 113, /** 81 **/
	KEYPAD_MOVEDNRIGHT = 114 /** 82 **/
	
	
	} KEYS;

int keypad_poll( void );
void keypad_init( void );
void keypad_set_leds( uint16_t mask );
char keypad_stop_pressed( void );
extern void keypadSet_Pressure_state( void);
extern void keypadSet_Speed_state( void);
#define leds_on()	do { PORTD &= ~(1 << 5); } while(0)
#define leds_off()	do { PORTD |=  (1 << 5); } while(0)



#endif
