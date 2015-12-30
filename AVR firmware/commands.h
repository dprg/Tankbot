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
Command -> from computer to Atmel
Response -> from Atmel to Computer

Motor odometry request command
ROD\m
    Odoemetry count for left and right motor response
    ODO 01234567 89ABCDEF\n

Sonar array measurement request command
RSN\n
    Sonar array measurement response
    SNR 012 345 678\n

Set left motor PWM command
LMT 012\n

Set right motor PWM command
RMT 012\n

Emergency stop command
XXX\n
    Generic command received response
    COK\n

Motor velocity request command
RVL\n
    Motor velocities response 
    VEL 01234567 89ABCDEF\n

*/


#ifndef _COMMANDS_H
#define _COMMANDS_H

void parse_RVL( char *line );
void parse_XXX( char *line );
void parse_LMT( char *line );
void parse_RMT( char *line );
void parse_RSN( char *line );
void parse_ROD( char *line );

#endif

