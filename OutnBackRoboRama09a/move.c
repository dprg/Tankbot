/****************************************************************************
*     Copyright (C) 2006-2007  Ed Paradis                                   *
*     Copyright (C) 2006-2007  Eric Sumner                                  *
*     Copyright (C) 2009 Paul Bouchier                                      *
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

/*
 * This is a total hack of outnback code to move forward or back, or rotate
 */
 
#include "tankbot.h"
#include "odo.h"
#include <stdio.h>
#include <math.h>
#include <sys/signal.h>
#include <ctype.h>


#ifdef SIMULATION
#define MAX_LOOPS 100
#else
#define MAX_LOOPS 5000
#endif

H_tankbot tb;
H_tankbot sensorComm;

#define PRINT_POS
#define PRINT_ERR

// constants
#define TWOPI 6.28318530717959
#define PI    3.14159265358979
#define DIR_CTRL_NONE 0
#define DIR_CTRL_DIFF 1
#define DIR_CTRL_COMPASS 2

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
float VELOCITY = 9.5;          
float TURN_SPEED = 5.0;

const float WAYPOINT_CLOSE = 6.0;      // inches
const float WAYPOINT_ARRIVED = 2.0;      // inches
const float TURN_DEADBAND = 4.0;        // degrees
const float TURN_RADIUS = 8.0;          // inches
void cleanup(int signum)
{
    tb_estop( tb );
    tb_estop( tb );
    tb_close( tb );
    exit(0);

}



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

void limit_pwm(float *pwm_l, float *pwm_r)
{
    *pwm_l = (*pwm_l > 1.0) ? 1.0 : *pwm_l;
    *pwm_l = (*pwm_l < -1.0) ? -1.0 : *pwm_l;
    *pwm_r = (*pwm_r > 1.0) ? 1.0 : *pwm_r;
    *pwm_r = (*pwm_r < -1.0) ? -1.0 : *pwm_r;

}
// calculate the wheel velocities
// speed is the speed we want to be moving at
// radius is the radius we want to be turning at
//   note that it can be negative and we turn the other way
// UNITS:
// speed: inches per second
// radius: inches
// command_vel_* : inches per second
float calc_velocities( float req_speed)
{
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
        command_vel_l = speed;
        command_vel_r = speed;

        // add in a component to correct difference between L & R tracks running at different speeds
        // if (req_speed > 0) {
        //     command_vel_l += 3*(ticks_to_inches(tb->r_pos) - ticks_to_inches(tb->l_pos));
        // } else {
        //     command_vel_r -= 3*(ticks_to_inches(tb->r_pos) - ticks_to_inches(tb->l_pos));
        // }
        return speed;
}

float prev_left_distance = 0.0;
float prev_right_distance = 0.0;

// pass a way point 'absolute' position
void update_navigation( float X_target, float Y_target  )
{
        float left_distance, right_distance;    // wheel distances (inches)
        float distance;         // how far we've traveled (inches)
        float theta = 0;            // how much we're turned (radians )
        float xd,yd;            // delta from where we are to the target (inches )
        
        // first we update the tankbot data struct so our data is 'current'
        tb_update_odometry( tb ); 
        tb_read_heading(sensorComm);

        // next we calculate the distance we've traveled thus far
        left_distance = ticks_to_inches(tb->l_pos) - prev_left_distance;
        right_distance = ticks_to_inches( tb->r_pos) - prev_right_distance;
        distance = (left_distance + right_distance) / 2.0;

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
        //target_angle = (90.0 - (atan2(yd,xd)*(180.0/PI))) - theta*(180.0/PI);
        // we want to prevent wind up
        target_angle = unwind_angle( target_angle );

        // printf(" *** %f ***\n", 90.0 - (atan2(yd,xd) * 180.0/PI) );
}

int move(float waypoint_y, int dir_ctrl)
{
    int count = MAX_LOOPS;  // we don't want to run forever, so set a maximum number of iterations
    float pwm_l, pwm_r, pid_l, pid_r;    // temporary variables, mainly so we can print debug statements
    int l_r_offset, l_r_diff;
    float req_speed;
    int headingDelta;

    printf("move distance: %f\n", waypoint_y);

    if (waypoint_y < 0) {
        VELOCITY = -VELOCITY;
    }
    
    // set our PID constants (same for left and right wheels)
    if (dir_ctrl == DIR_CTRL_NONE) {
        k_p = 0.9;
        k_i = 00.0;
        k_d = 00.0;
        i_state_max = 100.0;
    } else if (dir_ctrl == DIR_CTRL_DIFF) {
        k_p = 0.05;
        k_i = 0.1;
        k_d = 00.0;
        i_state_max = 100.0;
    } else if (dir_ctrl == DIR_CTRL_COMPASS) {
        sensorComm->desiredHeading = sensorComm->heading;
        k_p = 0.05;
        k_i = 0.1;
        k_d = 00.0;
        i_state_max = 100.0;
    }

    update_navigation( 0, waypoint_y ); // update target_distance
    l_r_offset = tb->l_pos - tb->r_pos; // get the initial difference between L & R encoders for use in difference speed control

    do {
        update_navigation( 0, waypoint_y ); // update target_distance
        printf("tdis %-5.2g ", target_distance);
        req_speed = calc_velocities(VELOCITY); 

        if (dir_ctrl == DIR_CTRL_NONE) {
            // compute the left and right wheel speeds
            printf(" cvl %g cvr %g", command_vel_l, command_vel_r);

            // find what we need to set the PWM to from PID calculators
            pid_l = PID1( ticks_to_inches( tb->l_vel ) , command_vel_l );
            pid_r = PID2( ticks_to_inches( tb->r_vel ) , command_vel_r );
            printf(" pl %g pr %g", pid_l, pid_r);

            // convert from inches per second to pwm duty cycle (0 to 1.0)
            pwm_l = pid_l * PWM_SCALE;      
            pwm_r = pid_r * PWM_SCALE;

            limit_pwm(&pwm_l, &pwm_r);
            printf(" pwm_l: %2.2f; pwm_r %2.2f ", pwm_l, pwm_r);


        } else if (dir_ctrl == DIR_CTRL_DIFF) {

            l_r_diff = tb->l_pos - tb->r_pos - l_r_offset; // get the difference between L & R encoders

            // depending on which track is lagging, slow down the other track with a PID controller to hold delta distance at 0
            if (((l_r_diff > 0) && (req_speed > 0)) || ((l_r_diff < 0) && (req_speed < 0))) {
                pid_l = PID1(l_r_diff, 0);      // run PID on target encoder difference between L & R of 0
                i_state2 = 0.0;                 // clear out R PID
                pwm_l = req_speed * PWM_SCALE - pid_l/100;      // slow down L motor
                pwm_r = req_speed * PWM_SCALE;
            } else {
                pid_r = PID2(l_r_diff, 0);      // run PID on target encoder difference between L & R of 0
                i_state1 = 0.0;                 // clear out L PID
                pwm_l = req_speed * PWM_SCALE;
                pwm_r = req_speed * PWM_SCALE - pid_r/100;      // slow down R motor
            }

            limit_pwm(&pwm_l, &pwm_r);
            printf(" pwm_l: %2.2f; pwm_r %2.2f ", pwm_l, pwm_r);

        } else if (dir_ctrl == DIR_CTRL_COMPASS) {
            headingDelta = sensorComm->desiredHeading - sensorComm->heading;

            // convertheadingDelta to an angle +180 - -180 relative to heading
            if( headingDelta > 180 )
                    headingDelta -= 360;
            else if( headingDelta < -180 )
                    headingDelta += 360;
            printf("headingDelta: %d ", headingDelta);

            // set motors based on which way we need to turn
            if (headingDelta > 0) {
                pid_l = PID1(headingDelta, 0);      // run PID on target encoder difference between L & R of 0
                i_state2 = 0.0;                 // clear out R PID
                pwm_l = req_speed * PWM_SCALE - pid_l/100;      // slow down L motor
                pwm_r = req_speed * PWM_SCALE;
            } else {
                pid_r = PID2(headingDelta, 0);      // run PID on target encoder difference between L & R of 0
                i_state1 = 0.0;                 // clear out L PID
                pwm_l = req_speed * PWM_SCALE;
                pwm_r = req_speed * PWM_SCALE - pid_r/100;      // slow down R motor
            }

            // limit pwm to -1 to +1
            limit_pwm(&pwm_l, &pwm_r);
            printf(" pwm_l: %2.2f; pwm_r %2.2f ", pwm_l, pwm_r);
        }

        // set the motor PWMs
        tb_set_motor( tb, TB_LEFT, pwm_l );
        tb_set_motor( tb, TB_RIGHT, pwm_r );

        // what our encoders think we've traveled
        printf(" l_pos %ld r_pos %ld ", tb->l_pos, tb->r_pos);      
        printf(" L=%-5.2f R=%-5.2f ", 
               ticks_to_inches(tb->l_pos), 
               ticks_to_inches(tb->r_pos) );

        // what our navigation thinks we've traveled
        printf(" Y=%-5.2f\n", Y);

        // stop when we start to get further away from target
        if (previous_target_distance < target_distance)
            break;
        previous_target_distance = target_distance;

        count--;        // iteration counter

        if (target_distance < WAYPOINT_ARRIVED || count < 0) {
            VELOCITY = 0;
        }

    } while ( VELOCITY != 0 || command_vel_l != 0 );  // stop when there's no call to move more

    // we're done, so make sure we're stopped
    tb_estop( tb );
    tb_estop( tb );
    return 0;
}

// turnAngle > 0 => turn clockwise (like boats)
int turn(int turnAngle)
{
    int count = MAX_LOOPS;  // we don't want to run forever, so set a maximum number of iterations
    float pwm_l, pwm_r, pid_l, pid_r;    // temporary variables, mainly so we can print debug statements
    int headingDelta;

    printf("turn angle: %d, ", turnAngle);

    // calculate new desired heading
    sensorComm->desiredHeading = sensorComm->heading + turnAngle;
    if (sensorComm->desiredHeading > 359) {
        sensorComm->desiredHeading -= 360;
    } else if (sensorComm->desiredHeading < 0) {
        sensorComm->desiredHeading += 360;
    }
    printf("desired heading: %d\n", sensorComm->desiredHeading);


    do {
        update_navigation( 0, 0 ); // update target_distance and target_angle
        printf("heading %d ", sensorComm->heading);

        headingDelta = sensorComm->desiredHeading - sensorComm->heading;

        // convertheadingDelta to an angle +180 - -180 relative to heading
        if( headingDelta > 180 )
                headingDelta -= 360;
        else if( headingDelta < -180 )
                headingDelta += 360;
        printf("headingDelta: %d ", headingDelta);
        printf("l_pos %ld r_pos %ld\n", tb->l_pos, tb->r_pos);      

        // set motors based on which way we need to turn
        if (headingDelta < 0) {
            pid_l = TURN_SPEED;
            pid_r = -TURN_SPEED;
        } else {
            pid_l = -TURN_SPEED;
            pid_r = TURN_SPEED;
        }

        // convert from inches per second to pwm duty cycle (0 to 1.0)
        pwm_l = pid_l * PWM_SCALE;      
        pwm_r = pid_r * PWM_SCALE;

        // set the motor PWMs
        tb_set_motor( tb, TB_LEFT, pwm_l );
        tb_set_motor( tb, TB_RIGHT, pwm_r );

        count--;        // iteration counter

    } while ( abs(headingDelta) > TURN_DEADBAND && count > 0 );

    // we're done, so make sure we're stopped and close the port
    tb_estop( tb );
    tb_estop( tb );
    return 0;
}

void usage()
{
    fprintf(stderr, "Usage: [move | turn] [-d | -c] [-r arduino_COM_port_#] [-v avr_COM_port_#]  [-m max_pwm] <distance or angle>\n \
Precede negative distance or angle with --\n\
-d: control tracks during straight-line travel based on difference between encoders\n\
-c: control tracks during straight-line travel based on compass\n\
If neither -c or -c, no direction control during straight line travel\n");
}

int main( int argc, char *argv[] )
{ 
    float waypoint_y;   // we're only going to deal with one waypoint for now
    int turnAngle;
    char avr_portname[20];
    char arduino_portname[20];
    int rv;
    int i;
    int dir_ctrl = DIR_CTRL_NONE;

    // default port names for tankbot HW
    sprintf(avr_portname, "/dev/ttyS0");
    sprintf(arduino_portname, "/dev/ttyUSB0");

    if (argc < 2) {
        usage();
        exit(0);
    }
    while ((rv = getopt(argc, argv, "cdm:r:v:")) != -1)
            switch(rv)
            {
            case 'c':
                dir_ctrl = DIR_CTRL_COMPASS;
                break;
            case 'd':
                dir_ctrl = DIR_CTRL_DIFF;
                break;

            case 'm':     // max pwm 
                  if (!isdigit(*optarg)) {
                      printf("error: port must be a number: %s\n", optarg);
                      exit(-1);
                  }
                  VELOCITY = atof(optarg); 
                  break;

            case 'r':     
                  if (!isdigit(*optarg)) {
                      printf("error: port must be a number: %s\n", optarg);
                      exit(-1);
                  }
                  i = atoi(optarg) - 1; 
                  sprintf(arduino_portname, "/dev/ttyS%d", i);
                  break;

            case 'v':     
                if (!isdigit(*optarg)) {
                    printf("error: port must be a number: %s\n", optarg);
                    exit(-1);
                }
                i = atoi(optarg) - 1; 
                sprintf(avr_portname, "/dev/ttyS%d", i);
                break;

            case '?':     
            default:  
                  fprintf(stderr, "invalid option or argument in default: rv=%d\n", rv);
                  exit(-1);
            }
    
    signal(SIGINT, cleanup); // set signal handler to terminate on ^C

    printf("using avr port %s arduino port %s\n", avr_portname, arduino_portname);

    // initialize & check communication with the AVR board
    tb = tb_open(avr_portname);
    if (tb_estop(tb))
        printf("Robot motor controller is talking.\n");
    else {
        printf("Robot motor controller is not talking.\n");
        tb_close(tb);
        return -1;
    }

    // initialize & check communication with the Arduino board
    sensorComm = tb_open(arduino_portname);
printf("opened arduino\n");
    if (tb_estop(sensorComm))
        printf("Robot sensors controller is talking.\n");
    else {
        printf("Robot sensors controller is not talking.\n");
        tb_close(tb);
        return -1;

    }

    // robot is ready, clear odometry & initialize sensors
    tb_reset_odometry( tb );
    tb_reset_sensors(sensorComm);


    init_nav();

    if (strncmp(argv[0], "./move", 4) == 0) {   // FIXME - chop off preceding part of path
        // set the distance of our single waypoint
        waypoint_y = atof(argv[optind]);             // inches
        move(waypoint_y, dir_ctrl);
    } else if (strncmp(argv[0], "./turn", 4) == 0) {   // FIXME - chop off preceding part of path
        // set the angle to turn
        turnAngle = atoi(argv[optind]);
        turn(turnAngle);
    }

    tb_close( tb );

}

