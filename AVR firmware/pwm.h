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

#ifndef _PWM_H
#define _PWM_H


// initializes the ports and timers for PWMing the motors
void pwm_init( void );

// set left (channel 0, motor A) PWM value given a signed  char
void set_motorA( signed char channel0 );
// set right ( channel 1, motor B) PWM given a signed char 
void set_motorB( signed char channel1);

#endif
