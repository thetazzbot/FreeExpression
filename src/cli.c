/*
* cli.c
*
* command line interface
*
* Passes data to the Language parser & interprets the results
*
* TODO: Re-implement scaling
* TODO: Implement store - or - cut feature
* TODO: Implement retrieve and cut feature
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
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <setjmp.h>
#include <math.h>
#include "timer.h"
#include "cli.h"
#include "usb.h"
#include "stepper.h"
#include "version.h"
#include "shvars.h"
#include "hpgl.h"
#include "g_code.h"
#include "display.h"
#include "keypad.h"

void cli_poll(void)
{
	STEPPER_COORD dstx, dsty;
	char c;
	int8_t cmd;
	uint8_t labelchar;
	
	
	while((c = (char)usb_getc())!=SERIAL_NO_DATA) 
	{
		switch(Lang)
		{
			case HPGL:
			cmd = hpgl_char(c, &dstx, &dsty, &labelchar);
			break;
			
			case G_CODE:
			cmd = gcode_char(c, &dstx, &dsty );
			break;
			
			//case GPGL:
			// TODO
			//break;
			
			default:
			continue;		// just consume everything and do nothing
			
		}
			

		switch(cmd) 
		{
			case CMD_PU:
			//sprintf(s,"PU: %d %d",dstx,dsty);
			//display_puts( s);
				stepper_move( dstx, dsty );
			break;
			case CMD_PD:
			//sprintf(s,"PD: %d %d",dstx,dsty);
			//	display_puts( s);
				stepper_draw( dstx, dsty );
			break;
		
			case CMD_INIT:
				// 1. home
				// 2. init scale etc
				// typically happens at start and end of each document;
				dstx = dsty = 0;				
		//	sprintf(s,"IN: %d %d",dstx,dsty);
			//display_puts( s);
				stepper_move( dstx, dsty );
				break;
			case CMD_SEEK0:
			//sprintf(s,"SK: %d %d",dstx,dsty);
			//display_puts( s);
				stepper_move( dstx, dsty  );
			break;
#ifdef non_supported_commands			
			case CMD_PA:  // not supported
			break;
			case CMD_ARCABS:  // not supported
				//arc_active = arc_init();
			break;
			case CMD_LB0:  // not supported
			//	text_beginlabel();
			break;
			case CMD_LB:  // not supported
			//	if (labelchar != 0) {
			//		char_active = text_char(labelchar, &dstx, &dsty, &penny);
			//	}
		
			break;
			case CMD_SI: // not supported
				//text_scale_cm(numpad[0], numpad[1]);
			break;
			case CMD_SR: // not supported
				//text_scale_rel(numpad[0], numpad[1]);
			break;
			case CMD_DI: // not supported
				//text_direction(numpad[0], numpad[1]);
			break;
			//case CMD_FS:
			//	stepper_pressure(numpad[0]*100);
			//	break;
			case CMD_VS:
			//sprintf(s,"VS: %d",(int)numpad[0]);
			//display_puts( s);
			//	stepper_speed(numpad[0]);
			break;
			case CMD_AS: // acceleration not supported
				//set_acceleration_mode(numpad[0]);
			break;
#endif
			default:
			break;
		}
	}
	
}
