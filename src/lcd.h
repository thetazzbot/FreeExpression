/*
 * lcd.h
 *
 * support for LCD module
 *
 *
 * This file is part of Freecut.
 *
 * Freecut is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2.
 *
 * Freecut is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Freecut. If not, see http://www.gnu.org/licenses/.
 *
 */

#ifndef LCD_H
#define LCD_H

#include <stdio.h>
#include <inttypes.h>

extern FILE lcd;

void lcd_init( void );
void lcd_position( unsigned char row, unsigned char col);		// 0 based row
int lcd_putchar( char c );

void lcd_puthex( uint8_t x );

#endif
