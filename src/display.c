/*
 * The display is composed of an UG-2864ASYDT01 OLED module using an SSD1325 display driver chip.
 * The graphics library used is u8glib
 *
 * https://code.google.com/p/u8glib/
 *
 * Device type can be either u8g_dev_ssd1325_nhd27oled_bw_sw_spi or the 2x variant, 
 * u8g_dev_ssd1325_nhd27oled_2x_bw_sw_spi
 *
 * https://code.google.com/p/u8glib/wiki/device
 *
 * The UG-2864ASYDT01 module uses mainly four pins to interface with the AVR:
 *
 *  SCK: PORTF, Bit 6
 *  MOSI: PORTF, Bit 5
 *  CS: PORTE, Bit 7
 *  A0: PORTF, Bit 7
 *
 * On the motherboard of the Cricut Expression, the 10 pin header has the following
 * connections:
 *
 * Pin 	|  Display	| AVR
 *------+-------+---------
 *  1   |  GND 	   | NC
 *  2   |  Vcc     | NC
 *  3   |  D1/SDIN | PF5
 *  4   |  D0/SCLK | PF6
 *  5   |  DC#     | PF7
 *  6   |  NC	   | PE4
 *  7   |  NC	   | PE5
 *  8   |  RES	   | PE6
 *  9   |  CS#	   | PE7
 *  10  |  NC      | 
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

/*
 TODO:  Replace the u8glib, it is adding so much lag to the UI.  I need to hand-craft
 a faster interface.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include "u8g.h"
#include "display.h"

u8g_t u8g;
static uint8_t cur_x=0,cur_y=0;

void display_firstpage(void)
{
	u8g_FirstPage(&u8g);
}

int display_nextpage(void)
{
	return u8g_NextPage(&u8g);
}
void display_init(void)
{
  /* select minimal prescaler (max system speed) */
  CLKPR = 0xFF;
                                                           // SCK    MOSI      CS       D/C
   u8g_InitSPI(&u8g, &u8g_dev_ssd1325_nhd27oled_2x_gr_sw_spi, PN(5, 6), PN(5, 5), PN(4, 7),PN(5,7), PN(4,6));
   u8g_SetRot180(&u8g);
   
   /* assign default color value */
   u8g_SetColorIndex(&u8g, 3);         /* max intensity */
 //  u8g_SetColorIndex(&u8g, 1);         /* pixel on */

  u8g_SetFont(&u8g, u8g_font_profont11);
  u8g_SetFontRefHeightExtendedText(&u8g);
  u8g_SetDefaultForegroundColor(&u8g);
  u8g_SetFontPosTop(&u8g);
  

}

/**
* Displays a string using u8glib's "picture loop".  There is
* significant lag happening here.
*/
void display_puts(char *s) {
	display_firstpage();
	do {
		u8g_DrawStr(&u8g, cur_x, cur_y, s);
	} while(display_nextpage());
	
}

/*
* Horrible code that needs to go away and be replaced by something better
*/

void display_println(char *s) {
	display_puts(s);
	cur_y+=10;
	cur_x=0;
}
int display_putc( char c)
{
	char buf[2];
	buf[1]=0x00;
	buf[0]=c;
	if(c=='\r') {
		cur_x++;
	}
	if(c=='\n') {
		cur_y=0;
	}
    u8g_SetFont(&u8g, u8g_font_6x10);	
	u8g_DrawStr(&u8g, cur_x, cur_y, buf);
	cur_y++;
	
	return 1;
}

