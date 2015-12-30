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

// Ed Paradis

/* EncoderHandler
 *
 * This handler keeps track of two quadrature inputs.
 *
 * X is connected to INT0 and PC0
 * Y is connected to INT1 and PC1
 *
 * The ISR directly increments and decrements the encoder counts, which are
 * signed longs (32 bits)
*/


#ifndef __ENCODERS_H
#define __ENCODERS_H

// call this to set up the interupt handlers, set pin directions, and zero globals
void encoders_init( void); 

//  turn off interrupts
void encoders_stop( void);


#endif
