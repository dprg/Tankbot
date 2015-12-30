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

// includes

#include <string.h>
#include <avr/io.h>

#include "serial.h"
#include "commands.h"
#include "util.h"
#include "encoders.h"
#include "pwm.h"

// globals

// prototypes

// implimentations

//////////////////////////// main() //////////////////////////////////////

int main( void )
{
    char line[64];  // a buffer to hold the incoming line

    // init uart
    usart_init();

    // init PWM generation
    pwm_init();

    // init sonar array

    // init encoder timer
    encoders_init();

    // enable interrupts
    sei();

    // signal start up
    usart_puts( "OK\n\r" );

    // PA6 and PA7 are debug LEDs
    DDRA |= (1<< PA7) | (1<<PA6);

    while (1)
    {
        // block for a '\n' terminated line, or 80 chars. which ever comes 1st
        usart_getline(line, 64);

        // debug
        //usart_puts( line);

        // compare the first three chars to known commands
        if( strncmp( line, "XXX", 3 ) == 0)
        {
            parse_XXX( line );
        }
        else if( strncmp( line, "ROD", 3 ) == 0 )
        {
            parse_ROD( line );
        } 
        else if ( strncmp( line, "LMT", 3 ) == 0)
        {
            parse_LMT( line );  // parse the values after the command, take
                                //  appropriate action, and send a response
        } 
        else if ( strncmp( line, "RMT", 3 ) == 0)
        {
            parse_RMT( line );
        } 
        else if ( strncmp( line, "RVL", 3 ) == 0)
        {
            parse_RVL( line );
        }
        else if ( strncmp( line, "RSN", 3 ) == 0)
        {
            parse_RSN( line );
        }
        //else
        //    usart_puts( line );
    }

    return 0;
}

