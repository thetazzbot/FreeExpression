/*
  serial.c - Low level functions for sending and recieving bytes via the serial port
  Part of Grbl v0.9

  Copyright (c) 2012-2014 Sungeun K. Jeon

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/
/* 
  This file is based on work from Grbl v0.8, distributed under the 
  terms of the MIT-license. See COPYING for more details.  
    Copyright (c) 2009-2011 Simen Svale Skogsrud
    Copyright (c) 2011-2012 Sungeun K. Jeon
*/ 

#include <avr/interrupt.h>
#include "serial.h"


uint8_t serial_rx_buffer[RX_BUFFER_SIZE];
uint8_t serial_rx_buffer_head = 0;
volatile uint8_t serial_rx_buffer_tail = 0;

uint8_t serial_tx_buffer[TX_BUFFER_SIZE];
uint8_t serial_tx_buffer_head = 0;
volatile uint8_t serial_tx_buffer_tail = 0;


#ifdef ENABLE_XONXOFF
  volatile uint8_t flow_ctrl = XON_SENT; // Flow control state variable
#endif
  

// Returns the number of bytes used in the RX serial buffer.
uint8_t serial_get_rx_buffer_count()
{
  uint8_t rtail = serial_rx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_rx_buffer_head >= rtail) { return(serial_rx_buffer_head-rtail); }
  return (RX_BUFFER_SIZE - (rtail-serial_rx_buffer_head));
}


// Returns the number of bytes used in the TX serial buffer.
// NOTE: Not used except for debugging and ensuring no TX bottlenecks.
uint8_t serial_get_tx_buffer_count()
{
  uint8_t ttail = serial_tx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_tx_buffer_head >= ttail) { return(serial_tx_buffer_head-ttail); }
  return (TX_BUFFER_SIZE - (ttail-serial_tx_buffer_head));
}


void serial_init()
{
  // Set baud rate
  #if BAUD_RATE < 57600
    uint16_t UBRR0_value = ((F_CPU / (8L * BAUD_RATE)) - 1)/2 ;
    UART1_STATUS &= ~(1 << U2X1); // baud doubler off  - Only needed on Uno XXX
  #else
    uint16_t UBRR0_value = ((F_CPU / (4L * BAUD_RATE)) - 1)/2;
    UART1_STATUS |= (1 << U2X1);  // baud doubler on for high baud rates, i.e. 115200
  #endif
  UBRR1H = UBRR0_value >> 8;
  UBRR1L = UBRR0_value;
            
  // enable rx and tx
  UART1_CONTROL |= 1<<RXEN1;
  UART1_CONTROL |= 1<<TXEN1;
	
  // enable interrupt on complete reception of a byte
  UART1_CONTROL |= 1<<RXCIE1;
	  
  // defaults to 8-bit, no parity, 1 stop bit
}


// Writes one byte to the TX serial buffer. Called by main program.
// TODO: Check if we can speed this up for writing strings, rather than single bytes.
void serial_write(uint8_t data) {
  // Calculate next head
  uint8_t next_head = serial_tx_buffer_head + 1;
  if (next_head == TX_BUFFER_SIZE) { next_head = 0; }

  // Wait until there is space in the buffer
  while (next_head == serial_tx_buffer_tail) { 

  }

  // Store data and advance head
  serial_tx_buffer[serial_tx_buffer_head] = data;
  serial_tx_buffer_head = next_head;
  
  // Enable Data Register Empty Interrupt to make sure tx-streaming is running
  UART1_CONTROL |=  (1 << UART1_UDRIE); 
}


// Data Register Empty Interrupt handler
ISR(UART1_TRANSMIT_INTERRUPT)
{
  uint8_t tail = serial_tx_buffer_tail; // Temporary serial_tx_buffer_tail (to optimize for volatile)
  
  #ifdef ENABLE_XONXOFF
    if (flow_ctrl == SEND_XOFF) { 
      UART1_DATA = XOFF_CHAR; 
      flow_ctrl = XOFF_SENT; 
    } else if (flow_ctrl == SEND_XON) { 
      UART1_DATA = XON_CHAR; 
      flow_ctrl = XON_SENT; 
    } else
  #endif
  { 
    // Send a byte from the buffer	
    UART1_DATA = serial_tx_buffer[tail];
  
    // Update tail position
    tail++;
    if (tail == TX_BUFFER_SIZE) { tail = 0; }
  
    serial_tx_buffer_tail = tail;
  }
  
  // Turn off Data Register Empty Interrupt to stop tx-streaming if this concludes the transfer
  if (tail == serial_tx_buffer_head) { UART1_CONTROL &= ~(1 << UART1_UDRIE); }
}


// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read()
{
  uint8_t tail = serial_rx_buffer_tail; // Temporary serial_rx_buffer_tail (to optimize for volatile)
  if (serial_rx_buffer_head == tail) {
    return SERIAL_NO_DATA;
  } else {
    uint8_t data = serial_rx_buffer[tail];
    
    tail++;
    if (tail == RX_BUFFER_SIZE) { tail = 0; }
    serial_rx_buffer_tail = tail;

    #ifdef ENABLE_XONXOFF
      if ((serial_get_rx_buffer_count() < RX_BUFFER_LOW) && flow_ctrl == XOFF_SENT) { 
        flow_ctrl = SEND_XON;
        UART1_CONTROL |=  (1 << UART1_UDRIE); // Force TX
      }
    #endif
    
    return data;
  }
}


ISR(UART1_RECEIVE_INTERRUPT)
{
  uint8_t data = UART1_DATA;
  uint8_t next_head;
  
  // Pick off runtime command characters directly from the serial stream. These characters are
  // not passed into the buffer, but these set system state flag bits for runtime execution.
      next_head = serial_rx_buffer_head + 1;
      if (next_head == RX_BUFFER_SIZE) { next_head = 0; }
    
      // Write data to buffer unless it is full.
      if (next_head != serial_rx_buffer_tail) {
        serial_rx_buffer[serial_rx_buffer_head] = data;
        serial_rx_buffer_head = next_head;    
        
        #ifdef ENABLE_XONXOFF
          if ((serial_get_rx_buffer_count() >= RX_BUFFER_FULL) && flow_ctrl == XON_SENT) {
            flow_ctrl = SEND_XOFF;
            UART1_CONTROL |=  (1 << UART1_UDRIE); // Force TX
          } 
        #endif
      //TODO: else alarm on overflow?
  }
}


void serial_reset_read_buffer() 
{
  serial_rx_buffer_tail = serial_rx_buffer_head;

  #ifdef ENABLE_XONXOFF
    flow_ctrl = XON_SENT;
  #endif
}
