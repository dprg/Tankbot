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
#include <stdio.h>
#include <math.h>
#include <sys/signal.h>

H_tankbot tb;

#define PRINT_POS
#define PRINT_ERR

// constants
#define TWOPI 6.28318530717959
#define PI    3.14159265358979

// globals (yuk)
float target_angle;                     // the angle to the target  (degrees)
float target_distance;          // the distance to the target (inches )
float k_p, k_i, k_d, i_state_max;               // the PID constants
float d_state1 = 0.0, i_state1 = 0.0;   // the PID states
float d_state2 = 0.0, i_state2 = 0.0;   // the PID states
float command_vel_l, command_vel_r;     // what we command our wheels to move at (inches)
float speed_array[5];
float previous_target_distance;
float X=0.0, Y=0.0;                     // how much we've traveled in X and Y (inches)

// laziness
int waypoint_count = 0;

// global constants (could also be defines)
const float VELOCITY = 1.0;
const float WAYPOINT_CLOSE = 6.0;      // inches
const float WAYPOINT_ARRIVED = 2.0;      // inches
const float TURN_DEADBAND = 5.0;        // degrees
const float TURN_RADIUS = 8.0;          // inches
const float PWM_SCALE = 1.0 / 700.0;    // converts inches per second to duty 
const float DISTANCE = 8.0;
                                                                                // cycle. it is the reciprical of the 
                                                                                // speed of the tank when commanded at 
                                                                                // 100% PWM
                                                                                // 714 for single battery pack, unloaded

// initialize navigation variables
void init_nav()
{
        int i;
    i_state1 = 0.0;
    i_state2 = 0.0;
    d_state1 = 0.0;
    d_state2 = 0.0;
    for (i=0; i<5; i++) speed_array[i]=0.0;
    previous_target_distance = 1e6;
    X = 0.0;
    Y = 0.0;                          

}
// given an angle in degrees, correct it to be between -180 and 180
float unwind_angle( float x )
{
        while( x > 180 )
                x -= 360;
        while( x < -180 )
                x += 360;
        return x;
}

void cleanup()
{
    tb_estop( tb );
    tb_estop( tb );
    tb_close( tb );
    exit(0);

}


// pass a way point 'absolute' position
void update_navigation( float X_target, float Y_target  )
{
        float left_distance, right_distance;    // wheel distances (inches)
        float distance;         // how far we've traveled (inches)
        
        // first we update the tankbot data struct so our data is 'current'
        tb_update_odometry( tb );
        tb_update_odometry( tb );

        // next we calculate the distance we've traveled thus far
        left_distance = ticks_to_inches(tb->l_pos) ;
        right_distance = ticks_to_inches( tb->r_pos) ;
        distance = (left_distance + right_distance) / 2.0;

        // calculate distance from robot location to target location
        target_distance = Y_target - distance;

}

int main( int argc, char *argv[] )
{ 
    float power = 0.1;

    // we're only going to deal with one waypoint for now
    float waypoint_x, waypoint_y;   


    // initialize communication with the AVR board
    tb = tb_open("/dev/ttyS0");

    // make sure we've got communications with the AVR board
    if (tb_estop(tb))
        printf("Robot is talking.\n");
    else {
        printf("Robot is not talking.\n");
        tb_close(tb);
        return -1;
    }

    signal(SIGINT, cleanup); // set signal handler to terminate on ^C

    // robot is ready, clear odometry
    tb_reset_odometry( tb );
    printf("reset odo done\n");

    // set the coordinates of our single waypoint
    waypoint_x = 0.0;             // inches
    waypoint_y = DISTANCE; // inches

    // set our PID constants (same for left and right wheels)
    k_p = 20.0;
    k_i = 20.0;
    k_d = 00.0;
    i_state_max = 100.0;
    init_nav();

    do {
        // update target_distance and target_angle
        update_navigation( waypoint_x, waypoint_y );
        printf("tdis %g", target_distance);

        // set the motor PWMs
        tb_set_motor( tb, TB_LEFT, power );
        tb_set_motor( tb, TB_RIGHT, power );

        // what our encoders think we've traveled
        printf(" ( L=%f R=%f\n )", 
               ticks_to_inches(tb->l_pos), 
               ticks_to_inches(tb->r_pos) );

        // stop when we start to get further away from target
/*         if (abs(previous_target_distance) < abs(target_distance))  */
/*             break;                                                 */
/*         previous_target_distance = target_distance;                */

        if (((target_distance < 0) && (power > 0)) || ((target_distance > 0) && (power < 0)))  {
            printf("sleeping before reverse\n");
            tb_estop(tb);
            tb_estop(tb);
            sleep(20);
            power = -power;
            previous_target_distance = power * 1e6; // large +ve or -ve value based on power
            if (power > 0)
                waypoint_y = DISTANCE;
            else
                waypoint_y = 0.0;
        }


    } while ( 1 );

    // we're done, so make sure we're stopped and close the port

    tb_estop( tb );
    tb_estop( tb );
    tb_close( tb );

    return 0;
}

