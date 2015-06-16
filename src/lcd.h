/*
 * lcd.h
 *
 * support for LCD module
 *
 *
 * Copyright 2010 <freecutfirmware@gmail.com> 
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
void lcd_backlight_on( void );
void lcd_backlight_off( void );
int lcd_putchar( char c, FILE *stream );
int lcd_getchar( FILE *stream );
void lcd_pos( uint8_t pos );
void lcd_puthex( uint8_t x );

#endif
