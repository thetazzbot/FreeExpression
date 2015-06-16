/*
 * flash.c
 *
 * Driver for serial dataflash AT45DB041D on main board, attached as follows:
 *
 * Pin 	|  Func	| AVR
 *------+-------+---------
 *  1   |  SI 	| PB7
 *  2   |  SCK  | PB1 
 *  4   |  !CS  | PB4 
 *  8   |  SO   | PB0 
 *
 * only contains a test/dump function for now.
 *
 * The AT45DB041B is an SPI compatible serial interface Flash memory ideally suited
 * for a wide variety of digital voice-, image-, program code- and data-storage applications.
 * Its 4,325,376 bits of memory are organized as 2048 pages of 264 bytes each. In
 * addition to the main memory, the AT45DB041B also contains two SRAM data buffers
 * of 264 bytes each.
 *
 * http://www.atmel.com/images/doc3443.pdf
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

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "flash.h"
#include "display.h"

#define MISO	(1 << 0)
#define SCK	(1 << 1)
#define CS	(1 << 4)
#define MOSI 	(1 << 7)

#define cs_low( )	do { PORTB &= ~CS; } while(0)
#define cs_high( )	do { PORTB |=  CS; } while(0)
#define mosi_low( )	do { PORTB &= ~MOSI; } while(0)
#define mosi_high( )	do { PORTB |=  MOSI; } while(0)
#define sck_low( )	do { PORTB &= ~SCK; } while(0)
#define sck_high( )	do { PORTB |=  SCK; } while(0)
#define get_miso( )	(PINB & 1)

#define DF_READY_FLAG 0x80 //Ready/busy status is indicated using bit 7 of the status register. 0x80 = b10000000

#define DF_STATUS 		0xd7 // status byte 

// read ops
#define DF_READPAGE		0xd2 // Main Memory Page Read
#define DF_READBUF1		0xd4 // Buffer 1 Read
#define DF_READBUF2		0xd6 // Buffer 2 Read
#define DF_CONTREAD	 	0xe8 // continuous array read (legacy) 

// buffer trf
#define DF_TRFBUF1		0x53 // Main Memory Page to Buffer 1 Transfer
#define DF_TRFBUF2		0x55 // Main Memory Page to Buffer 2 Transfer

// write ops
#define DF_WRITEBUF1	0x84 // Buffer 1 Write
#define DF_WRITEPAGE1	0x88 // Buffer 1 to Main Memory Page Program without Built-in Erase
#define DF_WRITEPAGE2E	0x86 // Buffer 1 to Main Memory Page Program with Built-in Erase
#define DF_WRITEBUF2	0x87 // Buffer 2 Write
#define DF_WRITEPAGE2	0x89 // Buffer 2 to Main Memory Page Program without Built-in Erase
#define DF_WRITEPAGE2E	0x86 // Buffer 2 to Main Memory Page Program with Built-in Erase
#define DF_PAGEERASE	0x81 // Page Erase
#define DF_BLOCKERASE	0x50 // Block Erase
#define DF_SECTORERASE	0x7C // Sector Erase
#define DF_PAGEPGM1		0x82 // Main Memory Page Program Through Buffer 1
#define DF_PAGEPGM2		0x85 // Main Memory Page Program Through Buffer 2

 
const uint8_t DF_pagebits[] PROGMEM ={9,  9,  9,  9,  9,  10,  10,  11};
const uint16_t DF_pagesize[] PROGMEM ={264,264, 264, 264, 264, 528, 528,1056};
#define DF_RESERVED_PAGES 0 // we will reserve the first two pages for status info

// always start at first page past reserved area
static unsigned int pageCounter = DF_RESERVED_PAGES;
static unsigned int byteCounter = 0;

static unsigned char pageBits = 0;
static unsigned int  pageSize = 264;  // init method will figure this out for us
static unsigned int  maxPages=2048; // init method will figure this out for us
static unsigned char bufferNumber = 1;

/***
* As it is now, the flash code implements methods to read and write bytes/pages/buffers
* It is used by the serial routine to buffer in all the incoming data, so that 
* processing can be delayed.  Once the cutting starts, the buffer is then read back
* by the hgpl system and parsed.
***/





/*
 * write a single byte to flash chip
 * not to be confused with writing data to the buffers
 */
static void flash_send_byte( uint8_t data )
{
    char i;

    for( i = 0; i < 8; i++ )
    {
        if( data & 0x80 )
	    mosi_high( );
	else
	    mosi_low( );
		sck_high( );
		data <<= 1;
		sck_low( );
    }
}

/*
 * read a single byte from flash chip
 */
static uint8_t flash_read_byte( void )
{
    uint8_t data = 0;
    char i;

    for( i = 0; i < 8; i++ )
    {
		sck_high( );
		data = (data << 1) + get_miso( );
		sck_low( );
    }
    return data;
}
static void flash_wait_ready(void)
{
	/*
	To read the status register, the CS pin must be asserted and the
	opcode of D7H must be loaded into the device. After the opcode is clocked in, the 1-byte status
	register will be clocked out on the output pin (SO), starting with the next clock cycle. The data in
	the status register, starting with the MSB (bit 7), will be clocked out on the SO pin during the next
	eight clock cycles. After the one byte of the status register has been clocked out, the sequence
	will repeat itself (as long as CS remains low and SCK is being toggled).

	Ready/busy status is indicated using bit 7 of the status register. If bit 7 is a 1, then the device is
	not busy and is ready to accept the next command. If bit 7 is a 0, then the device is in a busy
	state. Since the data in the status register is constantly updated, the user must toggle SCK pin to
	check the ready/busy status.
	*/
	cs_low( );
	flash_send_byte( DF_STATUS );
	while(!(flash_read_byte()  & DF_READY_FLAG)) {
		;
	}
	cs_high( );
	
}


static void flash_write_to_buffer(unsigned char bufferNo, unsigned int bufferAdr, uint16_t bytes,uint8_t *data)
{
	
	cs_low();
	cs_high();
	if (1 == bufferNo)
	{
		flash_send_byte(DF_WRITEBUF1);
		flash_send_byte((unsigned char)(bufferAdr >> (16 - pageBits)));
		flash_send_byte((unsigned char)(bufferAdr << (pageBits - 8)));
		flash_send_byte(0x00);
	}
	else
	{
		flash_send_byte(DF_WRITEBUF2);
		flash_send_byte((unsigned char)(bufferAdr >> (16 - pageBits)));
		flash_send_byte((unsigned char)(bufferAdr << (pageBits - 8)));
		flash_send_byte(0x00);
	}
	cs_low();
	cs_high();
	flash_wait_ready();
	
}

static void flash_buffer_to_page(unsigned char bufferNo, unsigned int pageNo)
{
	
}
// sequentially writes a byte to the data flash
// always starts at byte 0, page 2
uint8_t flash_write_next_byte(uint8_t data)
{	
	// make sure we don't over flow page count
	if(pageCounter>=maxPages)	{
		return 0; // no bytes written
	}
	// write the buffer to the flash, move to next page
	if(byteCounter == pageSize) {
		flash_buffer_to_page(bufferNumber,pageCounter);

		bufferNumber = (bufferNumber==1)?2:1;
		pageCounter++;
		byteCounter=0;
	} else {
		flash_write_to_buffer(bufferNumber,byteCounter,1,&data);
	}
	return 1; // one byte written ok
	

}


/*
 * provide 'count' dummy clocks
 */
static void flash_clocks( char count )
{
    while( --count >= 0 )
    {
        sck_high( );
		sck_low( );
    }
}


/*
 * read the Flash status byte
 */
static uint8_t flash_read_status( void )
{
    uint8_t status;
    cs_low( );
    flash_send_byte( DF_STATUS );
    status = flash_read_byte( );
    cs_high( );
    return status;
}

/*
 * write a command, consisting of 8 cmd bits, and 24 address bits.
 */
static void flash_write_cmd( uint8_t cmd, uint32_t addr )
{
    cs_low( );
    flash_send_byte( cmd );
    flash_send_byte( addr >> 24 );
    flash_send_byte( addr >> 16 );
    flash_send_byte( addr >>  8 );
}

uint8_t flash_read_next_byte( void )
{
	if(byteCounter==maxPages * pageSize) {
		cs_high();
		return 0x00;
	}
	return flash_read_byte();
}

/*
* Initializes the continuous read operation
*/
void flash_start_read(uint32_t offset)
{
    flash_write_cmd( DF_CONTREAD, offset);
}
void flash_start_write(uint32_t offset)
{
		
}
void flash_flush(void)
{
	cs_high();
	pageCounter=DF_RESERVED_PAGES;
	byteCounter=0;
}
void flash_init( void )
{
    DDRB |= MOSI | SCK | CS;
    uint8_t status=flash_read_status( );
	unsigned char index_copy;
	index_copy = ((status & 0x38) >> 3);	//get the size info from status register
	pageBits   = pgm_read_byte(&DF_pagebits[index_copy]);	//get number of internal page address bits from look-up table
	pageSize   = pgm_read_word(&DF_pagesize[index_copy]);   //get the size of the page (in bytes)
	
}

void flash_test(void)
{
	display_puts("Flash test");	
	flash_init();
	flash_start_read(4);
	char c[20];
	
	int i=0;
	for(i=0;i<4;i++) c[0]=flash_read_next_byte();
	for(i=0;i<20;i++) {
		c[i]=0x00;	
	}
	for(i=0;i<10;i++) {
		c[i]=flash_read_next_byte();
	}
	display_puts(c);
	flash_init();
}
