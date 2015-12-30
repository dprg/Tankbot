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

// ed paradis
// feb 2006

// just the utility functions, since they had grown to be fairly long

#include "util.h"
#include "serial.h"

////////////////////////////////////////////////////////////////////////////
// print an unsigned 32bit number
void usart_print_u32( unsigned long A)
{
	char buffer[10];	// where we'll store the digits
	signed char i;
	unsigned char allZeros;

	for (i=9; i>=0 ; i--)	// we're going backwards
	{
		buffer[i] = '0' + ( A % 10);	// see page 5 in notebook 
		A = A / 10;
	}

	// the buffer is populated in order, so push out each char
	allZeros = 1;	// assume zeros to start with
	for (i=0; i<10; i++)
	{
		if ( (buffer[i] == '0') && (allZeros == 1 ))
		{
			allZeros = 1;
			if( i==9)
				usart_putch( buffer[i] );
		} else
		{
			allZeros = 0;
			usart_putch( buffer[i] );
		}
	}
}

void usart_print_s32( signed long A)
{
	if ( A < 0 )
	{
		usart_putch( '-');
		usart_print_u32( -A );
	} else
		usart_print_u32( A);
}

// print an unsigned 16bit number
void usart_print_u16( unsigned short A)
{
	char buffer[5];	// where we'll store the digits
	signed char i;

	for (i=4; i>=0 ; i--)	// we're going backwards
	{
		buffer[i] = '0' + ( A % 10);	// see page 5 in notebook 
		A = A / 10;
	}

	// the buffer is populated in order, so push out each char
	for (i=0; i<5; i++)
	{
		usart_putch( buffer[i] );
	}
}

void usart_print_s16( signed short A)
{
	if ( A < 0 )
	{
		usart_putch( '-');
		usart_print_u16( -A );
	} else
		usart_print_u16( A);
}


// print an unsigned 8bit number
void usart_print_u8( unsigned char A)
{
	char buffer[3];	// where we'll store the digits
	signed char i;
    unsigned char allZeros;

	for (i=2; i>=0 ; i--)	// we're going backwards
	{
		buffer[i] = '0' + ( A % 10);	// see page 5 in notebook 
		A = A / 10;
	}

	// the buffer is populated in order, so push out each char
//	for (i=0; i<3; i++)
//	{
//		usart_putch( buffer[i] );
//	}
	// the buffer is populated in order, so push out each char
	allZeros = 1;	// assume zeros to start with
	for (i=0; i<3; i++)
	{
		if ( (buffer[i] == '0') && (allZeros == 1 ))
		{
			allZeros = 1;
			if( i==2)
				usart_putch( buffer[i] );
		} else
		{
			allZeros = 0;
			usart_putch( buffer[i] );
		}
	}
}



///////////////////////////////////////////////////////////////////////////////

// takes a character of hex ('0' thru 'F') and returns its decimal value (0-15)
// theres probably some bit fiddling way to do this but this is fine for now
unsigned char hex_to_dec( unsigned char A)
{
	if ( A >= '0' && A <= '9')
		return A - '0';
	else if ( A >= 'A' && A <= 'F' )
		return A - 'A' + 10;
	else if ( A >= 'a' && A <= 'f' )
		return A - 'a' + 10;
	else
		return 0;
}

