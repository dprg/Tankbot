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

#ifndef _SERIAL_H
#define _SERIAL_H

#include <avr/io.h>
#include <avr/interrupt.h>

// prototypes

void usart_init( void );
void usart_puts( char *data );
void usart_getline( char *line, int max);

void usart_putch( char data);
#endif
