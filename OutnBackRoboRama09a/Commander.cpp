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

/******************************************************************************
* This class is responsible for defining the goals that other classes should  *
* try to achieve, for example, goto waypoint. It may be informed of changes   *
* or issues by lower-level classes and choose to change or abandon goals      *
* prior to their completion. It handles goal changes in response to obstacles *
* and other world features. It runs the frames at the rate specified, calling *
* each object that needs to be called each frame                              *
*                                                                             *
* It is akin to the commander (captain) of a ship, who delegates steering,    *
* engine control, navigation, etc. to subordinates, but decides what the      *
* ship should try to accomplish, and handles exception conditions that        *
* require goal changes.                                                       *
*                                                                             *
******************************************************************************/

#include "Commander.h"

Commander::Commander(Navigator *nav_in)
{
	nav = nav_in;
}

int Commander::init()
{
	nav->init();
	return 0;
}
int Commander::run(int program)
{
	// Here's where the meat of the program running needs to be
	return 0;
}
