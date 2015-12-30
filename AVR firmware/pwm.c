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
// feb 2007 

#include "pwm.h"
#include <avr/io.h>
#include <stdlib.h>

// initializes the ports and timers for PWMing the motors
void pwm_init( void )
{
	// set some pins as outputs
	// OC1A is PD5 (pin 19)
	// OC1B is PD4 (pin 18)
	DDRD |= 0x30;	//0011 0000

	// we use A0 and A1 as the direction lines
	DDRA |= 0x03;	// 0000 0011
	PORTA &= ~( 1<<PA0 | 1<<PA1);

	// set up timers to generate PWM at about 10khz or so
	//TCCR1A = 0b10100001;	// 1010      clear on match, set on top (larger match values = longer pulses) pg110
							//     00    dont explicitly set output pins
							//       01  mode 5 (pg 111)
	//TCCR1B = 0b00001011;	// 0		no noise cancler
							//  0		input caputure edge select (ignored)
							//   0		reserved, must be zero
							//    01	mode 5, page 111
							//      011	prescaler, pg112-113, 011 means 1/64
	// mode 5: fast PWM, 8bit, TOP = 0x00FF, 
	TCCR1A = 0xA1;
	TCCR1B = 0x0B;

	// channel A value
	OCR1A = 0x0000;		// init to zero

	// channel B value, same as channel A
	OCR1B = 0x0000;		// init to zero

}

// set left (channel 0, motor A) PWM value given a signed  char
void set_motorA( signed char channel0 )
{
	// if the lower msb is set, we're going backwards
	if ( channel0 < 0 )		// left going backwards
	{
		// set the direction pin 
		PORTA |= (1 << PA0);
		// set the inverse of the PWM value
		OCR1A = 2 * ( 127 - abs( channel0 ) ) ;
	} 
    else	// left is going forwards, or stopped
	{
		// reset direction pin
		PORTA &= ~ ( 1<< PA0);
		// PWM is normal
		OCR1A = 2 * channel0;
	}
}

// set right ( channel 1, motor B) PWM given a signed char 
void set_motorB( signed char channel1)
{
    if ( channel1 < 0 )		// right going backwards
	{
		// set the direction pin
		PORTA |= (1 << PA1);
		// set the inverse of the PWM value
		OCR1B = 2 * ( 127 - abs( channel1 ) ) ;
	}
    else // right is going forwards, or stopped
	{
		// reset direction pin
		PORTA &= ~( 1<< PA1);
		// PWM is normal
		OCR1B = 2 * channel1 ;
	}

}

