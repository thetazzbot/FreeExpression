/*
 * usb.c
 *
 * The USB port is a UART, connected to a FTDI chip. 
 * This file is a wrapper on top of the low level serial routines found in serial.c
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
#include <avr/wdt.h>
#include <inttypes.h>
#include <stdio.h>
#include "usb.h"
#include "serial.h"


void usb_puts(const char *s )
{
      while (*s != '\0') {
	      serial_write(*s++);
      }
}

void usb_putc(uint8_t c)
{	
	 serial_write(c);
}

uint8_t usb_getc()
{
	return serial_read();
}

int usb_haschar(void)
{
	return serial_get_rx_buffer_count()>0;
}


/*
 * initialize USB UART. Assume fixed baudrate of 115200, and default 8N1.
 */
void usb_init( void ) 
{
	serial_init();
}
