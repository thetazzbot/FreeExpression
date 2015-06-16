/*
 * lcd.c
 * 
 * Driver for LCD module, based on standard Hitachi H44780, using a 
 * 10 pin connector for 4-bit only operation. Pinout of the connector
 * is as follows:
 *
 * Pin 	|  Func	| AVR
 *------+-------+---------
 *  1   |  GND 	| 
 *  2   |  Vcc  | 
 *  3   |  RS   | PF5
 *  4   |  R/!W | PF6
 *  5   |  E    | PF7
 *  6   |  D4	| PE4
 *  7   |  D5	| PE5
 *  8   |  D6	| PE6
 *  9   |  D7	| PE7
 * 10   | Light | PF4 (0=On, 1=Off)
 *
 * The display is 16x1 chars, but is actually wired as a 40x2 line
 * display, where the left half is the first line, and the right half
 * is the second line. Addresses for the chars are as follows:
 *
 * 00 01 02 03 04 05 06 07 | 40 41 42 43 44 45 46 47 
 * 
 * The lcd_putc()/lcd_pos() functions keep this into account,
 * and perform the conversion
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

#include <avr/io.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include "lcd.h"
#include "timer.h"

#define BACKLIGHT (1 << 4 )		// 
#define RS 	(1 << 5)		// Register Select
#define RW 	(1 << 6)		// Read/Write 
#define E  	(1 << 7)		// Enable (Clock)
#define DATA	(0xf0)			// Data bus mask

#define rw_h() 	do { PORTF |=  RW; } while(0)
#define rw_l() 	do { PORTF &= ~RW; } while(0)

#define e_h() 	do { PORTF |=  E;  } while(0)
#define e_l() 	do { PORTF &= ~E;  } while(0)

#define rs_h() 	do { PORTF |=  RS; } while(0)
#define rs_l() 	do { PORTF &= ~RS; } while(0)

uint8_t current_pos = 0;

FILE lcd = FDEV_SETUP_STREAM( lcd_putchar, 0, _FDEV_SETUP_WRITE );

uint8_t lcd_read( void )
{
    uint8_t val;

    PORTE |= DATA;	// enable pull ups
    DDRE &= ~DATA;	// DDRE[4:7] inputs
    rw_h( );		// read mode
    e_h( );		// two e_h()'s for timing
    e_h( );
    val = PINE & 0xf0;  // read MS nibble
    e_l( );
    e_h( );
    e_h( );
    val |= (PINE >> 4); // read LS nibble
    e_l( );
    rw_l( );		// write mode (default)
    DDRE |= 0xf0;	// PORTE[4:7] outputs
    return val;
}

void lcd_backlight_on( void )
{
    PORTF &= ~BACKLIGHT;
}

void lcd_backlight_off( void )
{
    PORTF |= BACKLIGHT;
}

/*
 * lcd_wait_ready: wait for busy flag (bit 7) 
 * to clear. Return -1 on timeout.
 */
int lcd_wait_ready( void )
{
    int i;

    for( i = 0; i < 10000; i++ )
    {
        if( !(lcd_read() & 0x80) )
	    return 0 ;
    }
    return -1;
}

/* 
 * write top 4 bits of 'val' to LCD module
 */
void lcd_write_nibble( uint8_t val )
{
    PORTE = (PORTE & ~DATA) | (val & DATA);
    e_h( );
    e_l( );
}

/*
 * write 'val' to a Hitachi control register
 */
void lcd_write_control( uint8_t val )
{
    lcd_write_nibble( val );
    lcd_write_nibble( val << 4 );
    lcd_wait_ready( );
}

/*
 * lcd_pos: set current position 0..15
 * 
 * Convert for split screen (position 8..15 must be mapped to 40..47)
 */
void lcd_pos( uint8_t pos )
{
    if( pos >= 16 )
        pos = 0;
    current_pos = pos;
    if( pos >= 8 )
        pos += 32;
    lcd_write_control( 0x80 + pos );
}

/*
 * write ASCII character to display
 */
int lcd_putchar( char c, FILE *stream )
{
    if( current_pos >= 16 )	
        return 1;
    rs_h();		// temporarily select data
    lcd_write_nibble( c );
    lcd_write_nibble( c << 4 );
    rs_l( );		// back to control 
    lcd_wait_ready( );
    // fix for split screen
    if( ++current_pos == 8 )
        lcd_pos( current_pos );
    return 1;
}

/* 
 * initialize the LCD module.
 *
 * Between function calls, RS, E, and RW are always kept 0.
 */
void lcd_init( void )
{
    PORTF &= ~(RW | E | RS);
    DDRF |= (RW |  E | RS | BACKLIGHT);
    DDRE |= DATA;;
    
    // 4-bit init sequence taken from Hitachi datasheet.

    msleep( 15 );	      // 15 ms delay to start
    lcd_write_nibble( 0x30 ); // 8 bit mode
    msleep( 5 );	      // > 4.1 ms delay
    lcd_write_nibble( 0x30 ); // 8 bit mode 
    usleep( 100 );	      // 100 us delay 
    lcd_write_nibble( 0x30 ); // 8 bit mode
    usleep( 100 );	      // 
    lcd_write_nibble( 0x20 ); // 4 bit mode, as 8 bit command
    usleep( 100 );	      // 

    // now we're in 4-bit mode, and the busy flag should work 

    lcd_write_control( 0x28 );// 4 bit, 2 lines, 5x8 dots
    lcd_write_control( 0x0c );// turn display on, cursor off
    lcd_write_control( 0x01 );// clear display, and go home
    current_pos = 0;	      // remember we're at home position
    lcd_backlight_on( );
} 

/* 
 * debugging functions, fairly quick so they can be used in ISR.
 */
void lcd_putnibble( uint8_t x )
{
    if( x < 10 )
        lcd_putchar( '0' + x, 0 );
    else
        lcd_putchar( 'a' + x - 10, 0 );
}

void lcd_puthex( uint8_t x )
{
    lcd_putnibble( x >> 4 );
    lcd_putnibble( x & 0x0f );
}
