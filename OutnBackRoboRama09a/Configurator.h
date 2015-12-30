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
using namespace std;      

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include "MotorControl.h"
#include "Compass.h"
#include "Odometer.h"
#include "Commander.h"
#include "Navigator.h"

class Configurator {
	MotorControl *mc;
	Sensor *comp;
	Sensor *odm;
	Navigator *nav;
	Commander *cmdr;

public:
	Commander* config(int configuration, H_tankbot tb, H_tankbot sensorComm);
};

#endif

