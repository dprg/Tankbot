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

#include <iostream>
#include "Configurator.h"


Commander* Configurator::config(int configuration, H_tankbot tb, 
                                                        H_tankbot sensorComm)
{
        int rv;

        cout<<"Configurator ctor";

        mc = new MotorControl(tb);
        comp = new Compass(sensorComm);
        odm = new Odometer(tb);
        nav = new Navigator(mc, comp);
        cmdr = new Commander(nav);

        rv = cmdr->init();

        if (rv == 0) {  
                return cmdr;    // successful initialization 
        } else {
                return NULL;
        }
}
