/*
 * lcd.c
 
 * Created: 6/15/2015 12:03:50 PM
 *  Author: GaryStofer@stofer.name
 * 
 * Driver for LCD module DIP204-4E 20x4, based on Samsung KS0073 controller chip or similar configured in SPI mode.
 * The 10 pin connector J1 and LCD module is re-wired for the following pin-out:
 *
 *J1 Pin|  LCD Pin	   | AVR
 *------+--------------+---------
 *  1   |  GND, 1 	   | 
 *  2   |  VCC, 2      | 5v
 *  3   |  MOSI,(R/W),5| PB2,MOSI,12,
 *  4   |  CS,(RS),4   | PB5,15 
 *  5   |  MISO,(D0),7 | PB3,MISO,13
 *  6   |  NC		   | PE4
 *  7   |  NC		   | PE5
 *  8   |  NC		   | PE6
 *  9   |  RESn,16	   | PE7
 * 10   |  SCK,(E),6   | PB1,SCK,11

The original connections to AVR pins 54-57 have been disconnected from J1 and are now connected to J5 
for JTAG debugging. 

The back light is permanently on and the contrast adjustment is fixed with a resistor. 
 *
 */

#include <avr/io.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include "lcd.h"
#include "spi.h"
#include "timer.h"

#define LCD_CSn PB5 // PB5

#define KS0073_START_BITS 0x1F		// start pattern 5 high bits
#define KS0073_DISPLAY_BIT (1<<6)	// Bit 7 low == command, high == text data
#define KS0073_READ_BIT   (1<<5)	// Bit 6 low == write, high == read
#define BUSY_FLAG_MASK	0x80

// The following 4 defines indicate the display memory address for column 0 on line 0 through 3
#define DD_ADDRESS_LN0	0x00
#define DD_ADDRESS_LN1	0x20
#define DD_ADDRESS_LN2	0x40
#define DD_ADDRESS_LN3	0x60

// File stream setup for use in fprintf()
FILE lcd = FDEV_SETUP_STREAM( lcd_putchar, 0, _FDEV_SETUP_WRITE );


/* This function reads the display in command mode  and waits until the Busy flag is not indicated
 * Returns the address counter (cursor position)
 */  

static unsigned char 
lcd_ks0073_wait_busy( void )
{
	BYTE data;
	int i = 0;
	cbi(PORTB,LCD_CSn);				// Chip select active
	spiSendByte(KS0073_START_BITS | KS0073_READ_BIT );	// start bit pattern: command read mode
	
	do 
	{
		if (++i > 10000 ) // to prevent a lockup if LCD fails 
			break;
		data = spiTransferByte( 0);	// reading lcd in command mode, D0-D6 == cursor position, need to transmit 0 to not retigger a start
	
	} while (data & BUSY_FLAG_MASK );
	
	sbi(PORTB,LCD_CSn);		// Chip select in-active
	return (data & ~BUSY_FLAG_MASK);
}


static void
lcd_ks0073_write( BOOL DISPLAY_cmd,  unsigned char data)
{
	BYTE b = KS0073_START_BITS; 
	
	lcd_ks0073_wait_busy( ); // wait until previous command is completed  
	
	cbi(PORTB,LCD_CSn);		// Chip select active
	
	if ( DISPLAY_cmd )		// or-in the display/command bit
		b |= KS0073_DISPLAY_BIT; 
		
	spiSendByte(b);
	b = data & 0xf;
	spiSendByte(b); // LS nibble first 
	b= data >> 4;
	spiSendByte(b); // MS nibble next
	
	sbi(PORTB,LCD_CSn);		// Chip select in-active

} 

/* 
 * initialize the SPI bus and LCD module.
 */
void lcd_init( void )
{
	sbi(DDRB,LCD_CSn);		// Set Chip select as output
	sbi(PORTB,LCD_CSn);		// not selected

	spiInit();				// configure SPI interface according to the requirements of the display interface 
    
	// initialization of KS0078 according to EA-DIP204-4 data sheet page3 
	lcd_ks0073_write( FALSE,0x34);	// Function set: 8Bit, set extension bit (RE)
	lcd_ks0073_write( FALSE,0x9);	// Ext Function set: 4 lines, 5 dot, normal cursor 
	lcd_ks0073_write( FALSE,0x30);	// Function set: 8Bit, clear extension bit (RE)
	lcd_ks0073_write( FALSE,0x1);	// Clear Display: Cursor at 1,1 ( takes 1.53ms)
	lcd_ks0073_write( FALSE,0x0C);  // Display ON/OFF: ON cursor OFF  
	lcd_ks0073_write( FALSE,0x6);	// Entry Mode set: Cursor auto increment 
	
	/*
	lcd_putchar('H');
	lcd_putchar('e');
	lcd_putchar('l');
	lcd_putchar('l');
	lcd_putchar('o');
	lcd_putchar( '\n' );
	
	for (BYTE i='A'; i<('A'+22); i++)
		lcd_ks0073_write(TRUE,i);
	*/
}

/* 
 * Function to position the cursor at a specific Row and Column of the LCD
 */
void 
lcd_position( unsigned char row, unsigned char col)
{
	BYTE pos = row*DD_ADDRESS_LN1 + col;
	
	if (col>=20 || row>=4)
		return;
		
	lcd_ks0073_write( FALSE,(0x80 | pos));
	
}

/*
 * write ASCII character to display
 */
int lcd_putchar( char c )
{
	BYTE pos;
	BYTE line;
	switch ( c )
 	{
		 case '\n':
			pos =  lcd_ks0073_wait_busy( );
			line = (pos / DD_ADDRESS_LN1) +1;
			if (line < 4)
				lcd_ks0073_write( FALSE,(0x80 | line*DD_ADDRESS_LN1 ));
			else // scroll up a line  
			{
				// The display has no built in vertical scroll capability
				// In order to scroll up the data memory would have to be read and and rewritten with scrolled content
				// not implemented!
				// Punt and simply clear the display and start from the top again
				lcd_ks0073_write( FALSE,0x1);	
			}
				
			break;
		 
		 case '\f':			// Clear Display: Cursor at 1,1 
		 	lcd_ks0073_write( FALSE,0x1);	
			break;
		 
		 case '\r':			// put cursor at beginning of current line 
		 	pos =  lcd_ks0073_wait_busy( );
			line = (pos / DD_ADDRESS_LN1);
			lcd_ks0073_write( FALSE,(0x80 | line*DD_ADDRESS_LN1 ));
			break;
		 
		 case '\b':			// backspace the cursor but don't write anything
			pos =  lcd_ks0073_wait_busy( );
			pos -= 1;
			lcd_ks0073_write( FALSE,(0x80 | pos ));	// fails when backing up beyond column 0 
			break;
		 
		 default:
			lcd_ks0073_write(TRUE,c);
	}
 
    return 0;	// needs to return 0 for fprintf
}

 
#ifdef Debuging
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
#endif
