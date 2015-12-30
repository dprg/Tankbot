/*****************************************************************************
*     Copyright (C) 2006-2008  Ed Paradis                                    *
*                                                                            *
*     This program is free software: you can redistribute it and/or modify   *
*     it under the terms of the GNU General Public License as published by   *
*     the Free Software Foundation, either version 3 of the License, or      *
*     (at your option) any later version.                                    *
*                                                                            *
*     This program is distributed in the hope that it will be useful,        *
*     but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*     GNU General Public License for more details.                           *
*                                                                            *
*     You should have received a copy of the GNU General Public License      *
*     along with this program.  If not, see <http://www.gnu.org/licenses/>,  *
*     or the LICENSE-gpl-3 file in the root directory of this repository.    *
*                                                                            *
*****************************************************************************/


/*
http://members.shaw.ca/climber/avrserial.html

*/

#include <avr/io.h>
#include <string.h>

#include "serial.h"

#define F_CPU 8250000
#define UART_BAUD_RATE 9600
// for slow speed uart (U2X = 0 ):
//#define UART_BAUD_CALC(UART_BAUD_RATE,F_CPU) ((F_CPU)/((UART_BAUD_RATE)*16l)-1)
// for high speed uart (U2X = 1 ):
#define UART_BAUD_CALC(UART_BAUD_RATE,F_CPU) ((F_CPU)/((UART_BAUD_RATE)*8l)-1)
void usart_init()
{
    /* set up serial port */
    UBRRH = (uint8_t)(UART_BAUD_CALC(UART_BAUD_RATE,F_CPU)>>8);
    UBRRL = (uint8_t)UART_BAUD_CALC(UART_BAUD_RATE,F_CPU);
    UCSRA |= (1<< U2X);

    UCSRB = (1 << RXEN) | (1 << TXEN);
    UCSRC = (1<< URSEL) | (3 << UCSZ0);
    
    // enable USART_DATA interrupt
    //UCSRB |= (1<< UDRIE );

    // turn on the recv interrupt
    UCSRB |= (1<<RXCIE);

}

/*

http://en.wikipedia.org/wiki/Circular_buffer
*/

#define BUFFER_SIZE 64 
char buffer[ BUFFER_SIZE ];
int input_index = 0;
int output_index = 0;

void usart_putch( char data)
{
    // convert \n to \r\n
    if( data == '\n')
        usart_putch( '\r');

    // increment the input index  (automatically wrapping to start as needed)
    input_index = (input_index + 1 ) % BUFFER_SIZE;

    // copy the data to where the input index is
    buffer[ input_index ] = data;
    
    UCSRB |= (1<<UDRIE);

}

ISR( SIG_USART_DATA)  // interrupt for tx data register empty
{
    // if buffer is not empty
    if ( ((input_index + BUFFER_SIZE - output_index) % BUFFER_SIZE) > 0 )
    {
        // increment the output index (and auto wrap)
        output_index = (output_index + 1) % BUFFER_SIZE;

        // copy data from where the output index is
        UDR = buffer[ output_index ] ;
    } 
    else // the buffer is empty
    {  
        // turn off interrupt
        UCSRB &= ~(1<< UDRIE );
    }

}

void usart_puts( char *data)
{
    //while not at the end of the input string
    int i = 0;
    while ( data[i] )
    {
        // if the buffer is not full
        if( ((input_index + BUFFER_SIZE - output_index) % BUFFER_SIZE) < (BUFFER_SIZE - 1) )
        {
            // put the next peice into the buffer
            usart_putch ( data[i]);
            i++;
        }
        else
        {
            // signal we're blocking to wait for space by turning on the
            //  interrupt to check for space in hardware tx 
            UCSRB |= (1<<UDRIE);
        }
    }
}

// AVR306: Using the AVR UART in C
// Routines for interrupt controlled USART
// Last modified: 02-06-21
// Modified by: AR
// translated for GCC by EP

/* UART Buffer Defines */
#define USART_RX_BUFFER_SIZE 128     /* 2,4,8,16,32,64,128 or 256 bytes */
#define USART_RX_BUFFER_MASK ( USART_RX_BUFFER_SIZE - 1 )
#if ( USART_RX_BUFFER_SIZE & USART_RX_BUFFER_MASK )
    #error RX buffer size is not a power of 2
#endif

/* Static Variables */
static unsigned char USART_RxBuf[USART_RX_BUFFER_SIZE];
static volatile unsigned char USART_RxHead = 0;
static volatile unsigned char USART_RxTail = 0;

ISR( SIG_UART_RECV )
{
    unsigned char data;
    unsigned char tmphead;

    /* Read the received data */
    data = UDR;                 
    /* Calculate buffer index */
    tmphead = ( USART_RxHead + 1 ) & USART_RX_BUFFER_MASK;
    USART_RxHead = tmphead;      /* Store new index */

    if ( tmphead == USART_RxTail )
    {
        /* ERROR! Receive buffer overflow */
        PORTA |= (1<<PA7);
    }
    
    USART_RxBuf[tmphead] = data; /* Store received data in buffer */
}

// blocking function to return the next character in the RX buffer
unsigned char usart_getch( void )
{
    unsigned char tmptail;
    
    while ( USART_RxHead == USART_RxTail )  /* Wait for incomming data */
        ;
    tmptail = ( USART_RxTail + 1 ) & USART_RX_BUFFER_MASK;/* Calculate buffer index */
    
    USART_RxTail = tmptail;                /* Store new index */
    
    return USART_RxBuf[tmptail];           /* Return data */
}

unsigned char DataInReceiveBuffer( void )
{
    return ( USART_RxHead != USART_RxTail ); /* Return 0 (FALSE) if the receive buffer is empty */
}

static volatile char egregious_hack[80];

// blocking function to wait until we've got max characters or we see a '\n'
void usart_getline( char *line, int max)
{
    int length = 0;
    char temp;

    do
    {
        temp = usart_getch();   // blocking get from RX buffer
        egregious_hack[length] = temp;    // store into buffer
        length++;
        if ( length == (max - 1) )  // we only have space for one more
                                    //  character in our buffer
        {
            length++; 
            egregious_hack[length] = '\0';   // append a '\0' to terminate the string

            strcpy( line, egregious_hack );

            return;  // break out of do...while
        }
    } while ( (temp != '\n') && (temp != '\r') ); 

    length++;
    egregious_hack[length] = '\0';

    strcpy( line, egregious_hack );

    return;
}


