/* 
 * HPGL state machine
 * Processes incoming bytes and sets up the state machine according to the previous
 * bytes received.
 *
 * It's important to have flow control here because we could easily overrun our tiny 
 * buffer and cause the state machine to go awol.
 *
 * Currently implemented is XON/XOFF but it is known that this is not fool proof.
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

#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include <avr/pgmspace.h>

#include "shvars.h"
#include "hpgl.h"
#include "scale.h"


int8_t pstate, nstate;

uint8_t etxchar;			///< End of text, default ^C

void hpgl_init() {
	pstate = STATE_EXP1;
	etxchar = 003;	// ^C
	translate_init();
}

int8_t hpgl_char(char c, STEPPER_COORD* x, STEPPER_COORD* y, uint8_t* lb) {
	static uint8_t i, si;
	static uint8_t numpad_idx;
	static int8_t cmd = CMD_ERR;
	static STEPPER_COORD fx, fy;
	
	*x = *y = -1;
	*lb = 0;
	
lolbel:	
	switch (pstate) {
		case STATE_EXP1:	// expect first letter 
			cmd = CMD_CONT;
			si = 0;
			
			switch (c) {
			case ' ': case '\n': case '\r' : case '\t':
						break;
			case 'P':	pstate = STATE_EXP_P;
						break;
			
			case 'S':	pstate = STATE_EXP_S;
						break;
			case 'I':	pstate = STATE_EXP_I;
						break;
			case 'A':	pstate = STATE_EXP_A;
						break;
			case 'L':	pstate = STATE_EXP_L;
						break;
			case 'D':	pstate = STATE_EXP_D;
						break;
			case 'V':	pstate = STATE_EXP_V;
						break;
			default:	pstate = STATE_SKIP_END;
						break;
			}
			break;
			
		case STATE_SKIP_END:
			if (c == ';') {
				pstate = STATE_EXP1;
				cmd = CMD_ERR;
			}
			break;
			
		case STATE_EXP_A:
			switch (c) {
			case ' ': case '\n': case '\r' : case '\t':
						break;
			case 'A':	// Arc Absolute
						numpad_idx = 0;
						pstate = STATE_EXP4;
						nstate = STATE_ARC;
						break;
			case 'S':	// Acceleration Select
						numpad_idx = 0;
						pstate = STATE_EXP4;
						nstate = STATE_AS;
						break;
			}
			break;

		case STATE_EXP_P:
			
			si = 0;
		
			switch (c) {
			case 'U':	cmd = CMD_PU;
						pstate = STATE_X;
						break;
			case 'D':	cmd = CMD_PD;
						pstate = STATE_X;
						break;
			case 'A':	cmd = CMD_PA;
						pstate = STATE_X;
						break;
			case 'G':	// PG: feed page/home
						*x = *y = 0;
						user_loc.x = user_loc.y = 0;
						cmd = CMD_PU;
						pstate = STATE_SKIP_END;
						break;
			case 'R':	
			case 'W':
			default:	cmd = CMD_ERR;
						pstate = STATE_SKIP_END;
						break;
			}
			break;
			
		case STATE_EXP_S:
			switch (c) {
			case 'P':	pstate = STATE_SP;	// select pen
						break;
			case 'C':	nstate = STATE_SC;
						pstate = STATE_EXP4;
						numpad_idx = 0;
						break;
			case 'I':	nstate = STATE_SI;		// SI: absolute character size in cm
						pstate = STATE_EXP4;
						numpad_idx = 0;
						break;
			case 'R':	nstate = STATE_SR;
						pstate = STATE_EXP4;
						numpad_idx = 0;
						break;
			default:
						pstate = STATE_SKIP_END;
						break;
			}
			break;
			
		case STATE_EXP_I:
			switch (c) {
			case 'P':	// IP: input coordinates
						nstate = STATE_IP;
						pstate = STATE_EXP4;
						numpad_idx = 0;
						break;
			case 'N':	// IN: init plotter
						pstate = STATE_EXP1;
						cmd = CMD_INIT;
						break;
			case 'H':	// IH: seek home (not HPGL)
						pstate = STATE_EXP1;
						cmd = CMD_SEEK0;
						break;
			default:
						pstate = STATE_SKIP_END;
						break;
			}
			break;
			
		case STATE_EXP_L:
			switch (c) {
			case 'B':	// LB: text label
						pstate = STATE_LB;
						cmd = CMD_LB0;
						break;
			default:
						pstate = STATE_SKIP_END;
						break;
			}
			break;
		
		case STATE_EXP_D:
			switch (c) {
			case 'T':	// DT: label terminator
						pstate = STATE_DT;
						break;
			case 'I':	// DI: label direction
						pstate = STATE_EXP4;
						nstate = STATE_DI;
						numpad_idx = 0;						
						break;
			default:
						pstate = STATE_SKIP_END;
						break;
			}
			break;
		
		case STATE_EXP_V:
			switch (c) {
			case 'S':	// VS: Velocity Select
						pstate = STATE_EXP4;
						nstate = STATE_VS;
						numpad_idx = 0;
						break;
			default:
						pstate = STATE_SKIP_END;
						break;
			}
			break;
			
		case STATE_SP:
			switch (c) {
			case ';':	pstate = STATE_EXP1;
						break;
			case '0':
						*x = *y = 0;
						user_loc.x = user_loc.y = 0;
						cmd = CMD_PU;
						pstate = STATE_SKIP_END;
						break;
			}
			break;
			
		case STATE_IP:	
			for (i = 0; i < 4; i++) ip_pad[i] = (int32_t)round(numpad[i]);
			pstate = STATE_EXP1;
			break;
						
		case STATE_SC:
			for (i = 0; i < 4; i++) sc_pad[i] = (int32_t)round(numpad[i]);
			translate_scale();
			pstate = STATE_EXP1;
			break;
			
		case STATE_SI:
			cmd = CMD_SI;
			pstate = STATE_EXP1;
			break;
						
		case STATE_SR:
			cmd = CMD_SR;
			pstate = STATE_EXP1;
			break;

		case STATE_X:
			switch (c) {	
			case ' ':
			case '\n':
			case '\r':
			case '\t':	break;
			case ',':	scratchpad[si++] = 0;
						fx=0;
						int ii=sscanf_P(scratchpad, PSTR("%d"), &fx);
						
						si = 0;
						pstate = STATE_Y;
						break;
			case ';':	cmd = CMD_ERR;
						pstate = STATE_EXP1;
						break;
			default:	scratchpad[si++] = c;
						break;
			}
			break;
			
		case STATE_Y:
			switch (c) {
			case ' ': case '\n': case '\r': case '\t':	
						break;
			case ',':	
			case ';':	scratchpad[si++] = 0;
						fy=0;
						sscanf_P(scratchpad, PSTR("%d"), &fy);
						si = 0;
						*y = fy;
						*x = fx;
						//userscale(fx, fy, x, y, &user_loc.x, &user_loc.y);
						break;
			default:	scratchpad[si++] = c;
						break;
			}
			
			switch (c) {
			case ',':	pstate = STATE_X;
						break;
			case ';':	pstate = STATE_EXP1;
						break;
			}
			break;
			
		case STATE_EXP4:
			switch (c) {
			case ' ': case '\n': case '\r': case '\t':	
						break;
			case ';':
			case ',':	scratchpad[si++] = 0;
						sscanf_P(scratchpad,PSTR("%d"),&fx);
						si = 0;
						numpad[numpad_idx] = fx;
						numpad_idx++;
						if (numpad_idx == 4 || c == ';') {
							pstate = nstate;
							goto lolbel;
						}
						break;
			default:	scratchpad[si++] = c;
						break;
			}
			break;

		case STATE_ARC:
			// AA xc, yc, CentralAngle [,Degrees per step]
			cmd = CMD_ARCABS;
			if (numpad_idx == 3) {
				numpad[3] = 5.0;
			}
			*x = *y = -1;
			pstate = STATE_EXP1;
			//for (i = 0; i < 4; i++) printf_P(PSTR("%f "), numpad[i]);
			break;
			
		case STATE_LB:
			// scanning text
			cmd = CMD_LB;
			if (c == etxchar) {
				pstate = STATE_EXP1;
				*lb = 000;
			} else {
				*x = *y = -1;
				*lb = c;
			}
			break;
			
		case STATE_DT:
			etxchar = c;
			pstate = STATE_EXP1;
			break;
			
		case STATE_DI:
			// numpad contains sin(theta), cos(theta)
			cmd = CMD_DI;
			pstate = STATE_EXP1;
			break;
			
		case STATE_VS:
			cmd = CMD_VS;
			pstate = STATE_EXP1;
			break;
		
		case STATE_AS:
			cmd = CMD_AS;
			pstate = STATE_EXP1;
			break;
	}
	
	return cmd;
}

