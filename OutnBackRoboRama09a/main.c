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


#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <sys/signal.h> 
#include <getopt.h>

#include "tankbot.h"
#include "odo.h"
#include "Commander.h"
#include "Configurator.h"

H_tankbot tb;
H_tankbot sensorComm;           // use the same tb_ routines for the arduino


// globals (yuk)

float target_distance;          // the distance to the target (inches )
float previous_target_distance;
float X=0.0;                     // how much we've traveled in X and Y (inches)


// global constants (could also be defines)
float VELOCITY = 10.0;          
const float WAYPOINT_CLOSE = 6.0;      // inches
const float WAYPOINT_ARRIVED = 2.0;      // inches

// initialize navigation variables
void init_nav()
{
    X = 0.0;

}


float prev_left_distance = 0.0;
float prev_right_distance = 0.0;

// pass a way point 'absolute' position
void update_navigation( float X_target )
{
        float left_distance, right_distance;    // wheel distances (inches)
        float distance;         // how far we've traveled (inches)
        float xd;            // delta from where we are to the target (inches )
        
        // first we update the tankbot data struct so our data is 'current'
        tb_update_odometry( tb );

        // next we calculate the distance we've traveled thus far
        left_distance = ticks_to_inches(tb->l_pos) - prev_left_distance;
        right_distance = ticks_to_inches( tb->r_pos) - prev_right_distance;
        distance = (left_distance + right_distance) / 2.0;

        // update step distances
        prev_left_distance = ticks_to_inches(tb->l_pos);
        prev_right_distance = ticks_to_inches(tb->r_pos);

        // accumulate our X translation distance
        X += distance;

        // calculate distance from robot location to target location
        xd = X_target - X;

}

void cleanup(int sig_num)
{
    tb_estop( tb );
    tb_estop( tb );
    tb_close( tb );
    exit(0);

}


void usage()
{
    fprintf(stderr, "Usage: drive -r arduino_COM_port_# -v avr_COM_port_#\n");
}

int main( int argc, char *argv[] )
{ 
    Commander *cmdr;
    char avr_portname[20];
    char arduino_portname[20];
    int rv;
    int i;

    // default port names for tankbot HW
    sprintf(avr_portname, "/dev/ttyS0");
    sprintf(arduino_portname, "/dev/ttyUSB0");

    if (argc < 2)
        usage();
    while ((rv = getopt(argc, argv, "r:v:")) != -1)
            switch(rv)
            {
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

    float waypoint; // we're only going to deal with one waypoint for now
    float vel;
        
    waypoint = atof(argv[1]);   // set coordinate of single waypoint in inches
    signal(SIGINT, cleanup); // set signal handler to terminate on ^C

        // make sure we've got communications with the AVR board
    tb = tb_open(avr_portname);  // initialize comms with AVR board
    if(tb_estop(tb))  
        printf("AVR board is talking.\n");
    else 
        {
        printf("AVR board is not talking.\n");
        tb_close(tb);
        return -1;
    }
    
        // make sure we've got communications with the Arduino board
        sensorComm = tb_open(arduino_portname); // initialize comms with Arduino board
    if(tb_estop(sensorComm))  
        printf("Arduino board is talking.\n");
    else
    {
        printf("Arduino is not talking.\n");
        tb_close(sensorComm);
        tb_close(tb);
        return -1;
    }
    
        // robot is ready, create & call the Configurator object, which creates &
        // initializes everything else, & returns a Commander object
        Configurator configurator;
        cmdr = configurator.config(1, tb, sensorComm );
        if (cmdr == NULL) {
                printf("Failed to initialize objects with configurator");
                return -1;
        }

        // Tell commander to run the program. It returns when it can't do any more
        rv = cmdr->run(1);

        exit(0);

        // everything beyond here needs to move into the classes

    init_nav();

        vel = VELOCITY;   // forward velocity

    do
    {
            // update target_distance and target_angle
            update_navigation( waypoint );

            

            

            // stop when we start to get further away from target
            if (previous_target_distance < target_distance)
                break;
                previous_target_distance = target_distance;
            
            // print debugging info
            // printf(" ***(  %ld %ld )  \n", tb->l_pos, tb->r_pos );


    } while ( target_distance > WAYPOINT_ARRIVED );

    // we're done, so make sure we're stopped and close the port

    tb_estop( tb );
    tb_estop( tb );

        return 0;

}

