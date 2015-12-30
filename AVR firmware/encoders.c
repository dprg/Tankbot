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

#include "encoders.h"
#include <avr/io.h>
#include <avr/interrupt.h> 
/*#include "c:\winavr\avr\include\avr\io.h"
#include "c:\winavr\avr\include\avr\interrupt.h"
#include "c:\winavr\avr\include\avr\signal.h" */

// the globals
signed long encoderX;   // raw encoder value
signed long encoderY;
signed long prev_encoderX;  // raw encoder value from last timer0 fire
signed long prev_encoderY;
signed long distanceX;  // distance traveled in ticks
signed long distanceY;
signed long velocityX;
signed long velocityY;

// call this to set up the interupt handlers, set pin directions, and zero globals
void encoders_init( void )
{
	// configure PD6 and PD7 as inputs
	DDRD &= ~( _BV(PD6) | _BV(PD7) );

	// set PD6 and PD7 to not have internal pullups
	//PORTD &= ~( _BV(PD6) | _BV(PD7) );
	// set PD6 and PD7 to have internal pullups
	PORTD |= ( _BV(PD6) | _BV(PD7) );

	// configure INT0 and INT1 as inputs
	DDRD &= ~( _BV(PD2) | _BV(PD3) );
	
	// set INT0 and INT1 to trigger on rising edge
	MCUCR |= 0x0F;

	// enable interrupts on INT0 and INT1
	GICR |= 0xC0;

	// zero out the globals
	encoderX = encoderY = 0;
    prev_encoderX = prev_encoderY = 0;
    distanceX = distanceY = 0;
    velocityX = velocityY = 0;

    // set up a timer0 to run every X milliseconds
    //  (1 / 8.5 MHz) * 1024 * 256 = 30.84 ms
    TCCR0 |= (1<<CS02) | (1<<CS00); // normal mode, no compare, 1/1024 prescaler
    
    // interrupt on overflow (8 bit counter overflow)
    TIMSK |= (1<<TOIE0);

}

//  turn off interrupts
void encoders_stop( void)
{
	// disable INT0 and INT1 as interrupts
	GICR &= ~0xC0;

    // turn off timer0
    TIMSK &= ~(1<<TOIE0);
	
}	

// triggers when INT0 goes high
//ISR(SIG_INTERRUPT0)
ISR(SIG_INTERRUPT0)
{
	if ( bit_is_set( PIND, 6) )		// test PD6
	{	// this means the out of phase line was already high when the in phase
		// line went high, therefore we're going backwards
		encoderX --;
	} else
	{
		// the out of phase line is not yet high when the in-phase line has
		// gone high. therefore we're going forwards
		encoderX ++;
	}
}

// similar to above, but for INT1
//ISR( SIG_INTERRUPT1)
ISR( SIG_INTERRUPT1)
{	
	if ( bit_is_set( PIND, 7) )		// test PD7
	{
		encoderY--;
	} else
	{
		encoderY++;
	}
}


// this should be called on regular intervals
ISR( SIG_OVERFLOW0)
{
    // calculate velocities
    velocityX = encoderX - prev_encoderX;
    prev_encoderX = encoderX;
    velocityY = encoderY - prev_encoderY;
    prev_encoderY = encoderY;

    // calculate distances
    distanceX += velocityX;
    distanceY += velocityY;

}
