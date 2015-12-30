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

#include <math.h>

#ifndef _ODO_H
#define _ODO_H

double ticks_to_inches( int ticks );
int inches_to_ticks( double inches );
double ticks_to_radians( int ticks );
int radians_to_ticks( double radians);

extern const double WHEELBASE_INCHES;
extern const double TICKS_PER_INCH;


#endif
