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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef TB_ESTOP_TIMEOUT
#define TB_ESTOP_TIMEOUT 0.25 
#endif

#ifndef TANKBOT_H
#define TANKBOT_H

// comment the following line out for running on Tankbot
//#define SIMULATION

#define TB_LEFT 0x01
#define TB_RIGHT 0x02


#define MAX_VELOCITY 10.0          // max inches per sec
#define PWM_SCALE  (1.0 / MAX_VELOCITY)    // converts inches per second to duty cycle. it is the reciprocal of the 100% PWM speed
#define LEFT_DRAG 0.9
#define RIGHT_DRAG 1.0

struct Tankbot {
        int fd_rd, fd_wr;
        long l_vel, r_vel;
        long l_zero, r_zero;
        long l_pos, r_pos;
        int  initialHeading;
        int  desiredHeading;
        int  heading;
};
typedef struct Tankbot* H_tankbot;

H_tankbot tb_alloc();
void tb_free( H_tankbot tb);
H_tankbot tb_open( const char *fname );
void tb_close( H_tankbot tb);
int tb_estop( H_tankbot tb);
int read_with_timeout( int fd_rd, char *input_buffer, int data_to_read_length, float timeout);

void tb_reset_odometry( H_tankbot tb);
void tb_update_odometry( H_tankbot tb);
void tb_set_motor( H_tankbot tb, int side, float power);
void tb_reset_sensors( H_tankbot sensorComm );
void tb_read_heading( H_tankbot sensorComm );

#endif
