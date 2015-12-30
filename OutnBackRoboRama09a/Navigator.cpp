/****************************************************************************
*     Copyright (C) 2009  Paul Bouchier                                     *
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


/**************************************************************************
* This class is responsible for knowing where it is on the Cartesian      *
* plane and for instrucint the robot to ackt in a way that will reach the *
* next waypoint (or other goal TBD) while accounting for issues that      *
* may require navigation to deal with, such ase turns to next waypoint,   *
* & calculating planned direction, & verifying that the last turn turned  *
* the right amount & not running into obstacles.                          *
*                                                                         *
* It is akin to a ship's navigator on the bridge, calling down to the     *
* engine room and telling the helmsman where to steer. The navigator has  *
* the binoculars to look for rocks, and the map and radar and GPS, to know*
* where the ship is.                                                      *
*                                                                         *
**************************************************************************/

#include "Navigator.h"

Navigator::Navigator(MotorControl *mc_in, Sensor *compass_in)
{
	compass = compass_in;
	mc = mc_in;
} 
int Navigator::init()
{
	int rv;
	rv = mc->init();
	return rv;
}
int Navigator::addWaypoint(int x, int y)
{
	return 0;
}
void Navigator::abort()
{

}        
