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
#ifdef MACHINE_EXPRESSION
#ifndef _KEYPAD_EXPRESSION_H
#define _KEYPAD_EXPRESSION_H

#define KBD_MAX_COLS	24
#define KBD_MAX_ROWS 	5
enum {
	// KEYPAD_STOP, // not a scanned key, its a reset
	// I know it's kind of dumb to put numbers next to enums but if we decide to rename them or move them around, the number should stay with the name.
	KEYPAD_F1 = 0,
	KEYPAD_F2 = 1,
	KEYPAD_1 = 2,
	KEYPAD_2 = 3,
	KEYPAD_3 = 4,
	KEYPAD_4 = 5,
	KEYPAD_5 = 6,
	KEYPAD_6 = 7,
	KEYPAD_7 = 8,
	KEYPAD_8 = 9,
	KEYPAD_9 = 10,
	KEYPAD_0 = 11,
	KEYPAD_SPACE = 12,
	KEYPAD_BACKSPACE = 13,
	KEYPAD_PORTRAIT = 14,
	KEYPAD_FLIP = 15,
	KEYPAD_MINUS = 16,
	KEYPAD_OK = 17,
	KEYPAD_PLUS = 18,
	KEYPAD_XTRA1 = 19,
	KEYPAD_F3 = 24,
	KEYPAD_F4 = 25,
	KEYPAD_Q = 26,
	KEYPAD_W = 27,
	KEYPAD_E = 28,
	KEYPAD_R = 29,
	KEYPAD_T = 30,
	KEYPAD_Y = 31,
	KEYPAD_U = 32,
	KEYPAD_I = 33,
	KEYPAD_O = 34,
	KEYPAD_P = 35,
	KEYPAD_CHARDISPLAY = 36,
	KEYPAD_RESETALL = 37,
	KEYPAD_FITPAGE = 38,
	KEYPAD_QUANTITY = 39,
	KEYPAD_XTRA2 = 43,
	KEYPAD_F5 = 48,
	KEYPAD_F6 = 49,
	KEYPAD_A = 50,
	KEYPAD_S = 51,
	KEYPAD_D = 52,
	KEYPAD_F = 53,
	KEYPAD_G = 54,
	KEYPAD_H = 55,
	KEYPAD_J = 56,
	KEYPAD_K = 57,
	KEYPAD_L = 58,
	KEYPAD_SEMICOLON = 59,
	KEYPAD_REPEATLAST = 60,
	KEYPAD_SOUNDONOFF = 61,
	KEYPAD_MIXMATCH = 62,
	KEYPAD_MULTICUT = 63,
	KEYPAD_MOVEUPLEFT = 64,
	KEYPAD_MOVEUP = 65,
	KEYPAD_MOVEUPRIGHT = 66,
	KEYPAD_LINERETURN = 67,
	KEYPAD_MATERIALSAVER = 72,
	KEYPAD_REALDIALSIZE = 73,
	KEYPAD_Z = 74,
	KEYPAD_X = 75,
	KEYPAD_C = 76,
	KEYPAD_V = 77,
	KEYPAD_B = 78,
	KEYPAD_N = 79,
	KEYPAD_M = 80,
	KEYPAD_COMMA = 81,
	KEYPAD_PERIOD = 82,
	KEYPAD_SLASH = 83,
	KEYPAD_LOADLAST = 84,
	KEYPAD_SETCUTAREA = 85,
	KEYPAD_FITLENGTH = 86,
	KEYPAD_CENTERPOINT = 87,
	KEYPAD_MOVELEFT = 88,
	KEYPAD_CUT = 89,
	KEYPAD_MOVERIGHT = 90,
	KEYPAD_SETTINGS = 91,
	KEYPAD_SHIFT = 96,
	KEYPAD_CUT_SHIFTLOCK = 97,
	KEYPAD_EQUALS = 100,
	KEYPAD_LEFTBRACKET = 101,
	KEYPAD_RIGHTBRACKET = 102,
	KEYPAD_LEFTBRACE = 103,
	KEYPAD_RIGHTBRACE = 104,
	KEYPAD_QUOTE = 105,
	KEYPAD_49 = 106,
	KEYPAD_50 = 107,
	KEYPAD_LOADMAT=108,
	KEYPAD_UNLOADMAT=109,
	KEYPAD_AUTOFILL = 110,
	KEYPAD_MATSIZE = 111,
	KEYPAD_MOVEDNLEFT = 112,
	KEYPAD_MOVEDN = 113,
	KEYPAD_MOVEDNRIGHT = 114
	} KEYS;
	
#endif
#endif //#ifdef MACHINE_EXPRESSION