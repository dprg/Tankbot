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
* This class is responsible for managing the motors in such a way as          * 
* to achieve the Navigator's commands, and for alerting the system if         *
* the commands cannot be executed.                                            *
*                                                                             *
* The drive command makes sure the tracks run the same distance when running  *
* straight, or the planned distance if turning. The motor PID cotnrollers are *
* instantiated in this class.                                                 *
*                                                                             *
* The return from a motion command indictes whether the the command was       *
* accepted. Command completion status is logged with completion parameters    *
* on the logger before the next command begins execution.                     *
*                                                                                                                                                         *
* It is akin to the engine room of a ship - the engineers don't know where    *
* the ship is going or what direction it's pointed, they just keep the engines*
* running properly, set speed on command.                                     *
******************************************************************************/

#include "MotorControl.h"
#include "pid.h"

MotorControl::MotorControl(H_tankbot tb_in)
{
        tb = tb_in;
        tb_reset_odometry( tb );   // reset the encoder counters
}

MotorControl::~MotorControl()
{
        tb_estop(tb);   // make sure motors are stopped
}

int MotorControl::init()
{
        return 0;
}

int MotorControl::drive(short req_speed, short radius)
{

        Pid trackDiffPid(20.0, 20.0, 0.0);
                // calculate each side
        command_vel_l = req_speed;
        command_vel_r = req_speed;

        // add in a component to correct difference between L & R tracks running at different speeds
        if (req_speed > 0) {
            command_vel_l += 3*(ticks_to_inches(tb->r_pos) - ticks_to_inches(tb->l_pos));
        } else {
            command_vel_r -= 3*(ticks_to_inches(tb->r_pos) - ticks_to_inches(tb->l_pos));
        }
                // convert from inches per second to pwm duty cycle (0 to 1.0)
                pwm_l = pid_l * PWM_SCALE;      
                pwm_r = pid_r * PWM_SCALE;

                // set the motor PWMs
                tb_set_motor( tb, TB_LEFT, pwm_l );
                tb_set_motor( tb, TB_RIGHT, pwm_r );

        return 0;
}

void MotorControl::abort()
{

}
