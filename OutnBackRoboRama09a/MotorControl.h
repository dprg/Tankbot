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

#include "tankbot.h"
#include "odo.h"

#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

class MotorControl {

	H_tankbot tb;
	float pwm_l, pwm_r, pid_l, pid_r;
	float command_vel_l, command_vel_r;     // what we command our wheels to move at (inches/sec)

public:
	MotorControl(H_tankbot tb);
	~MotorControl();
	int init();
	int drive(short speed, short radius);
	void abort();
};

#endif
