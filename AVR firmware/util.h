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

#include "serial.h"

#ifndef _UTIL_H
#define _UTIL_H

//////////////////////////// printing decimal numbers /////////////////////
void usart_print_u32( unsigned long A);
void usart_print_s32( signed long A);
void usart_print_u16( unsigned short A);
void usart_print_s16( signed short A);
void usart_print_u8 ( unsigned char A);

// takes a character of hex ('0' thru 'F') and returns its decimal value (0-15)
unsigned char hex_to_dec( unsigned char A);


#endif


