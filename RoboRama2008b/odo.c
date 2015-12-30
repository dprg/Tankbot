/****************************************************************************
*     Copyright (C) 2006-2007  Ed Paradis                                   *
*     Copyright (C) 2006-2007  Eric Sumner                                  *
*                                                                           *
*     This program is free software: you can redistribute it and/or modify  *
*     it under the terms of the GNU General Public License as published by  *
*     the Free Software Foundation, either version 3 of the License, or     *
*     (at your option) any later version.                                   *
*                                                                           *
*     This program is distributed in the hope that it will be useful,       *
*     but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*     GNU General Public License for more details.                          *
*                                                                           *
*     You should have received a copy of the GNU General Public License     *
*     along with this program.  If not, see <http://www.gnu.org/licenses/>, *
*     or the LICENSE-gpl-3 file in the root directory of this repository.   *
****************************************************************************/

#include "tankbot.h"
#include "odo.h"
#include <math.h>

//#define WHEELBASE_INCHES 12.5  // actual wheel base
//const double WHEELBASE_INCHES=18.0; // guessing
const double WHEELBASE_INCHES=9.0; // guessing
const double TICKS_PER_INCH=1800.0; // also guessing

double ticks_to_inches( int ticks )
{
	return (double) ticks / TICKS_PER_INCH;
}

int inches_to_ticks( double inches )
{
	return (int) TICKS_PER_INCH / inches;
}

double ticks_to_radians( int ticks )
{
	//double temp;
	//temp = atan2( ticks_to_inches(ticks) , WHEELBASE_INCHES );	
	return ticks_to_inches(ticks) / WHEELBASE_INCHES ;
}

int radians_to_ticks( double radians )
{
	return inches_to_ticks(radians * WHEELBASE_INCHES);
}



