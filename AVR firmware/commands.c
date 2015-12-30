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

#include <stdlib.h>

#include "commands.h"
#include "serial.h"
#include "util.h" 
#include "encoders.h"
#include "pwm.h"

/////////////////////// externs and globals /////////////////////////////
//volatile extern signed long encoderX;
//volatile extern signed long encoderY;
//volatile extern signed long prev_encoderX;  // raw encoder value from last timer0 fire
//volatile extern signed long prev_encoderY;
volatile extern signed long distanceX;  // distance traveled in ticks
volatile extern signed long distanceY;
volatile extern signed long velocityX;  // velocities via timer0
volatile extern signed long velocityY;

/////////////////////// parse functions /////////////////////////////////

/*
Motor velocity request command
RVL\n
    Motor velocities response
    VEL 01234567 89ABCDEF\n
*/
void parse_RVL( char *line )
{
    // get velocities from last update of timer (we dont want to have things
    //  change beneath our feet so we make locals)
    signed long velX = velocityX;
    signed long velY = velocityY;

    // print response
    usart_puts( "VEL " );
    usart_print_s32( velX );  // print real left velocity
    usart_puts( " ");
    usart_print_s32( velY );  // print real right velocity
    usart_puts( "\n");
}

/*
Emergency stop command
XXX\n
    Generic command received response
    COK\n
*/
void parse_XXX( char *line )
{
    // set both mother PWMs to zero
    set_motorA( 0 );
    set_motorB( 0 );

    // print response
    usart_puts( "COK\n");
}

/*
Set left motor PWM command
LMT 012\n
    Generic command received response
    COK\n
*/
void parse_LMT( char *line )
{
    int pwm = 0;

    // skip up to correct place in *line
    line += 4;

    // decode a signed char
    pwm = atoi( line );

    // set left PWM value
    set_motorA( pwm );

    // print response
    usart_puts( "COK\n");

}

/*
Set right motor PWM command
RMT 012\n
    Generic command received response
    COK\n
*/
void parse_RMT( char *line )
{
    int pwm;

    // skip up to correct place in *line
    line += 4;

    // decode a signed char
    pwm = atoi( line);

    // set right PWM value
    set_motorB( pwm );

    // print response
    usart_puts( "COK\n");

}

/*
Sonar array measurement request command
RSN\n
    Sonar array measurement response
    SNR 012 345 678\n
*/
void parse_RSN( char *line )
{
    // TODO initiate sonar sweep/measurement

    // print response
    usart_puts( "SNR ");
    usart_print_u8( 100 );  // TODO print real left sonar measurement
    usart_puts( " " );
    usart_print_u8( 75 );   // TODO print real middle sonar value
    usart_puts( " " );
    usart_print_u8( 33 );   // TODO print real right sonar value
    usart_puts( "\n" );

}

/*
Motor odometry request command
ROD\m
    Odoemetry count for left and right motor response
    ODO 01234567 89ABCDEF\n
*/
void parse_ROD( char *line )
{
    //  get latest odometry measurements
    signed long disX = distanceX;
    signed long disY = distanceY;

    // print response
    usart_puts( "ODO ");
    usart_print_s32( disX );   // left encoder value
    usart_puts( " " );
    usart_print_s32( disY );   // right encoder value
    usart_puts( "\n" );

}
