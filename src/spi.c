/*! \file spi.c \brief SPI interface driver. */
//*****************************************************************************
//
// File Name	: 'spi.c'
// Title		: SPI interface driver
// Author		: Pascal Stang - Copyright (C) 2000-2002
// Created		: 11/22/2000
// Revised		: 06/06/2002
// Version		: 0.6
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"

// Define the SPI_USEINT key if you want SPI bus operation to be
// interrupt-driven.  The primary reason for not using SPI in
// interrupt-driven mode is if the SPI send/transfer commands
// will be used from within some other interrupt service routine
// or if interrupts might be globally turned off due to of other
// aspects of your program
//
// Comment-out or uncomment this line as necessary
//#define SPI_USEINT

#ifdef SPI_USEINT
//  variables
static volatile u08 spiTransferComplete;

// SPI interrupt service handler
SIGNAL(SIG_SPI)
{
	spiTransferComplete = TRUE;
}
#endif

// access routines
void spiInit()
{
#ifdef __AVR_ATmega1281__
	// setup SPI I/O pins
	sbi(PORTB, 1);	// set SCK hi
	sbi(DDRB, 1);	// set SCK as output
	cbi(DDRB, 3);	// set MISO as input
	sbi(DDRB, 2);	// set MOSI as output
	sbi(DDRB, 0);	// SS must be output for Master mode to work
#elif __AVR_ATmega8__
    // setup SPI I/O pins
    sbi(PORTB, 5);  // set SCK hi
    sbi(DDRB, 5);   // set SCK as output
    cbi(DDRB, 4);   // set MISO as input
    sbi(DDRB, 3);   // set MOSI as output
    sbi(DDRB, 2);   // SS must be output for Master mode to work
#else
#pragma message( "No Matching device define for SPI configuration")
#endif

	// setup SPI interface :
	// master mode
	sbi(SPCR, MSTR);
	// clock = f/4
	// cbi(SPCR, SPR0);
	// cbi(SPCR, SPR1);
	// clock = f/16
	sbi(SPCR, SPR0);
	cbi(SPCR, SPR1);
	//Select Mode 3 
	sbi(SPCR, CPHA);
	sbi(SPCR, CPOL);
	// Data order LSB first
	sbi(SPCR,DORD);
	// enable SPI
	sbi(SPCR, SPE);
		
	// clear status
	inb(SPSR);
	

	// enable SPI interrupt
	#ifdef SPI_USEINT
	spiTransferComplete = TRUE;
	sbi(SPCR, SPIE);
	#endif
}
/*
void spiSetBitrate(u08 spr)
{
	outb(SPCR, (inb(SPCR) & ((1<<SPR0)|(1<<SPR1))) | (spr&((1<<SPR0)|(1<<SPR1)))));
}
*/
void spiSendByte(u08 data)
{
//	inb(SPSR);		// clear interrupt flag
	outb(SPDR, data);	// This also clears the Interrupt flag 
	
	// send a byte over SPI and ignore reply
#ifdef SPI_USEINT
		while(!spiTransferComplete);
		spiTransferComplete = FALSE;
#else
		while(!(inb(SPSR) & (1<<SPIF)));
#endif


}

u08 spiTransferByte(u08 data)
{
#ifdef SPI_USEINT
	// send the given data
	spiTransferComplete = FALSE;
	outb(SPDR, data);
	// wait for transfer to complete
	while(!spiTransferComplete);
#else
	// send the given data
	outb(SPDR, data);
	// wait for transfer to complete
	while(!(inb(SPSR) & (1<<SPIF)));
#endif
	// return the received data
	return inb(SPDR);
}

u16 spiTransferWord(u16 data)
{
	u16 rxData = 0;

	// send MS byte of given data
	rxData = (spiTransferByte((data>>8) & 0x00FF))<<8;
	// send LS byte of given data
	rxData |= (spiTransferByte(data & 0x00FF));

	// return the received data
	return rxData;
}
