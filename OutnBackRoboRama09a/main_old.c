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
const float VELOCITY = 60.0;
const float WAYPOINT_CLOSE = 6.0;      // inches
const float WAYPOINT_ARRIVED = 2.0;      // inches
const float TURN_DEADBAND = 5.0;        // degrees
const float TURN_RADIUS = 8.0;          // inches
const float PWM_SCALE = 1.0 / 700.0;    // converts inches per second to duty 
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

// returns a value in the units passed to it (inches)
// value is the current value of the variable to control
// target is the target value
float PID1( float value, float target )
{
        float error;
        float p, i, d;
        float ret;

        error = target - value;
        p = k_p * error;
        d = k_d * (error - d_state1 );
        d_state1 = error;
        i_state1 += error;

        // cap I term windup
        if( i_state1 > i_state_max )
                i_state1 = i_state_max;
        if( i_state1 < -i_state_max )
                i_state1 = -i_state_max;

        i = k_i * i_state1;

        ret = p + i + d;
        
        return ret;
}

// returns a value in the units passed to it (inches)
float PID2( float value, float target )
{
        float error;
        float p, i, d;
        float ret;

        error = target - value;
        p = k_p * error;
        d = k_d * (error - d_state2 );
        d_state2 = error;
        i_state2 += error;

        // cap I term windup
        if( i_state2 > i_state_max )
                i_state2 = i_state_max;
        if( i_state2 < -i_state_max )
                i_state2 = -i_state_max;

        i = k_i * i_state2;

        ret = p + i + d;
        
        return ret;
}

// calculate the wheel velocities
// speed is the speed we want to be moving at
// radius is the radius we want to be turning at
//   note that it can be negative and we turn the other way
// UNITS:
// speed: inches per second
// radius: inches
// command_vel_* : inches per second
void calc_velocities( float req_speed, float radius )
{
        float d = 0.5 * WHEELBASE_INCHES;       // distance from the center of the bot
                float speed;

                // calculate a moving average of the last 5 speed iterations as the new speed
                speed_array[4] = speed_array[3];
                speed_array[3] = speed_array[2];
                speed_array[2] = speed_array[1];
                speed_array[1] = speed_array[0];
                speed_array[0] = req_speed;
                speed = (speed_array[0] + speed_array[1] + speed_array[2] + speed_array[3]
                        + speed_array[4])/5;
                                                                                //  to the tread/wheel

        // TODO do something about the ambiguous case of radius == 0.0
        //  it means 'spin in place' but which way do we spin? left or right?

        // calculate each side
        command_vel_l = (2.0 * speed) / 
                                        (1 + (radius - d) / (radius + d));
        command_vel_r = (2.0 * speed) / 
            (1 + (radius + d) / (radius - d));

        // add in a component to correct difference between L & R tracks running at different speeds
        if (req_speed > 0) {
            command_vel_l += 3*(ticks_to_inches(tb->r_pos) - ticks_to_inches(tb->l_pos));
        } else {
            command_vel_r -= 3*(ticks_to_inches(tb->r_pos) - ticks_to_inches(tb->l_pos));
        }
}

float prev_left_distance = 0.0;
float prev_right_distance = 0.0;

// pass a way point 'absolute' position
void update_navigation( float X_target, float Y_target  )
{
        float left_distance, right_distance;    // wheel distances (inches)
        float distance;         // how far we've traveled (inches)
        float theta;            // how much we're turned (radians )
        float xd,yd;            // delta from where we are to the target (inches )
        
        // first we update the tankbot data struct so our data is 'current'
        tb_update_odometry( tb );
        tb_update_odometry( tb );

        // next we calculate the distance we've traveled thus far
        left_distance = ticks_to_inches(tb->l_pos) - prev_left_distance;
        right_distance = ticks_to_inches( tb->r_pos) - prev_right_distance;
        distance = (left_distance + right_distance) / 2.0;

        // calculate our turn angle (clipped to 360 degrees)
        //theta = (left_distance - right_distance) / WHEELBASE_INCHES;
        //theta -= (float) (int) (theta/TWOPI) *TWOPI;              // kludgy!!!
        // temp = (ticks_to_inches(tb->l_pos) - ticks_to_inches(tb->r_pos)) / WHEELBASE_INCHES;
        // theta = fmod( temp, TWOPI );
		// I don't think calculating theta is possible - there are two unknowns (theta & radius)
		// & two variables (L & R)
		theta = 0.0;

        printf("\nl_pos %ld r_pos %ld\n", tb->l_pos, tb->r_pos);      
        // printf("theta %f ", theta);

        // update step distances
        prev_left_distance = ticks_to_inches(tb->l_pos);
        prev_right_distance = ticks_to_inches(tb->r_pos);

        // accumulate our X and Y translation distances
        X += distance * sin( theta );
        Y += distance * cos( theta );

        // calculate distance from robot location to target location
        xd = X_target - X;
        yd = Y_target - Y;
        target_distance = sqrt((xd*xd)+(yd*yd));
        //target_distance = sqrt(yd*yd);

        // calculate bearing from current heading to target
        target_angle = (90.0 - (atan2(yd,xd)*(180.0/PI))) - theta*(180.0/PI);
        // we want to prevent wind up
        target_angle = unwind_angle( target_angle );

        // printf(" *** %f ***\n", 90.0 - (atan2(yd,xd) * 180.0/PI) );
}

int main( int argc, char *argv[] )
{ // we don't want to run forever, so set a maximum number of iterations
        int count = 0;

        // we're only going to deal with one waypoint for now
        float waypoint_x, waypoint_y;   

        // temporary variables, mainly so we can print debug statements
        float vel, turn, pwm_l, pwm_r, pid_l, pid_r;

        int i;
        char junk[10];

        printf("way points:\n");
        for( i=1; i < argc; i=i+2)
                printf("  ( %f , %f )\n", atof(argv[i]), atof(argv[i+1]) );

        // wait for key press
        fgets( junk, 1, stdin );

        // initialize communication with the AVR board
        tb = tb_open("/dev/ttyS0");

        // make sure we've got communications with the AVR board
        if(tb_estop(tb))
                printf("Robot is talking.\n");
        else
        {
                printf("Robot is not talking.\n");
                tb_close(tb);
                return -1;
        }
        
		// robot is ready, clear odometry
		tb_reset_odometry( tb );

        // set the coordinates of our single waypoint
        waypoint_x = atof(argv[1]);             // inches
        waypoint_y = atof(argv[2]);             // inches

        // set our PID constants (same for left and right wheels)
        k_p = 20.0;
        k_i = 20.0;
        k_d = 00.0;
        i_state_max = 100.0;
        init_nav();

        do
        {
                // update target_distance and target_angle
                update_navigation( waypoint_x, waypoint_y );
                printf("tdis %g tang %.1f", target_distance, target_angle);

                // forward velocity
                vel = VELOCITY;
                turn = TURN_RADIUS * 1e6;

                // printf(" vel %g trn %g", vel, turn);

                // compute the left and right wheel speeds
                calc_velocities( vel, turn );
                printf(" cvl %g cvr %g", command_vel_l, command_vel_r);

                // find what we need to set the PWM to from PID calculators
                pid_l = PID1( ticks_to_inches( tb->l_vel ) , command_vel_l );
                pid_r = PID2( ticks_to_inches( tb->r_vel ) , command_vel_r );
                printf(" pl %g pr %g", pid_l, pid_r);
                
                // convert from inches per second to pwm duty cycle (0 to 1.0)
                pwm_l = pid_l * PWM_SCALE;      
                pwm_r = pid_r * PWM_SCALE;

                // set the motor PWMs
                tb_set_motor( tb, TB_LEFT, pwm_l );
                tb_set_motor( tb, TB_RIGHT, pwm_r );

                // what our encoders think we've traveled
                printf(" ( L=%f R=%f )", 
                        ticks_to_inches(tb->l_pos), 
                        ticks_to_inches(tb->r_pos) );

                // what our navigation thinks we've traveled
                printf(" ( X=%f Y=%f )\n", X, Y);
                
                // stop when we start to get further away from target
                if (previous_target_distance < target_distance)
                    break;
                    previous_target_distance = target_distance;
                
                // print debugging info
                // printf(" ***(  %ld %ld )  \n", tb->l_pos, tb->r_pos );

                count++;        // iteration counter

        } while ( target_distance > WAYPOINT_ARRIVED && count < 10000 );

        // we're done, so make sure we're stopped and close the port

        tb_estop( tb );
        tb_estop( tb );

                printf("\n\n*** sleeping before reverse ***\n\n");
                sleep(1);

        // Now run backwards
        waypoint_x = 0 - atof(argv[1]);             // inches
        waypoint_y = 0 - atof(argv[2]) + 5;             // inches
		init_nav();

        do
        {

                // update target_distance and target_angle
                update_navigation( waypoint_x, waypoint_y );
                printf("tdis %g tang %.1f", target_distance, target_angle);

                // forward velocity
                vel = -VELOCITY;
                turn = TURN_RADIUS * 1e6;

                // printf(" vel %g trn %g", vel, turn);

                // compute the left and right wheel speeds
                calc_velocities( vel, turn );
                printf(" cvl %g cvr %g", command_vel_l, command_vel_r);

                // find what we need to set the PWM to from PID calculators
                pid_l = PID1( ticks_to_inches( tb->l_vel ) , command_vel_l );
                pid_r = PID2( ticks_to_inches( tb->r_vel ) , command_vel_r );
                printf(" pl %g pr %g", pid_l, pid_r);
                
                // convert from inches per second to pwm duty cycle (0 to 1.0)
                pwm_l = pid_l * PWM_SCALE;      
                pwm_r = pid_r * PWM_SCALE;

                // set the motor PWMs
                tb_set_motor( tb, TB_LEFT, pwm_l );
                tb_set_motor( tb, TB_RIGHT, pwm_r );

                // what our encoders think we've traveled
                printf(" ( L=%f R=%f )", 
                        ticks_to_inches(tb->l_pos), 
                        ticks_to_inches(tb->r_pos) );

                // what our navigation thinks we've traveled
                printf(" ( X=%f Y=%f )\n", X, Y);
                
                                // stop when we start to get further away from target
                                if (previous_target_distance < target_distance)
                                        break;
                                previous_target_distance = target_distance;
                
                // print debugging info
                // printf(" ***(  %ld %ld )  \n", tb->l_pos, tb->r_pos );

                count++;        // iteration counter
        } while ( target_distance > WAYPOINT_ARRIVED && count < 10000 );
        
        // we're done, so make sure we're stopped and close the port
        tb_estop( tb );
        tb_estop( tb );
        tb_close( tb );

        return 0;
}

