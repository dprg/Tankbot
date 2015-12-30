/****************************************************************************
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>    
#include <string.h>
#include <ctype.h>
#include <sys/select.h>

#define BUFLEN 80


void usage()
{
    fprintf(stderr, "Usage: tb_sim avr_COM_port_# arduino_COM_port_#\n");
}

void clr_buf(char buf[])
{
    for (int i=0; i<BUFLEN; i++) {
        buf[i] = '\0';
    }
}

void process_cmd(int port)
{
    char inbuf[BUFLEN];
    char outbuf[BUFLEN];
    char *bufptr;
    int rv;

    bufptr = inbuf;

    while((rv = read(port, bufptr, 80)) >= 0) {
        //printf("got %d bytes, message: %s\n", rv, bufptr);
        bufptr += rv;
        if (*(bufptr-1) == '\r') {
            *bufptr = '\0';
            printf("\nreceived command %s\n", inbuf);
            break;
        }
    }
    if (strncmp(inbuf, "XXX", 3) == 0) {
        sprintf(outbuf, "COK\n");
        rv = write(port, outbuf, strlen(outbuf));
        return;
    }
}

int main( int argc, char *argv[] )
{
        

        int     port1, port2;
        int     rv;
        char port1name[20];
        char port2name[20];
        fd_set rfds;

        if (argc < 3) {
            usage();
            exit(-1);
        }

        while ((rv = getopt(argc, argv, "p")) != -1)
            switch(rv)
            {
             case '?':     
             default:  
                  printf("invalid option or argument in default: rv=%d\n", rv);
                  exit(-1);
            }

        if (!isdigit(argv[optind][0])) {        // get first com port #
            printf("error: port must be a number: %s\n", argv[optind]);
            exit(-1);
        }
        sprintf(port1name, "/dev/ttyS%d", atoi(argv[optind])-1);
        if (!isdigit(argv[optind+1][0])) {        // get 2nd COM port #
            printf("error: port must be a number: %s\n", argv[optind+1]);
            exit(-1);
        }
        sprintf(port2name, "/dev/ttyS%d", atoi(argv[optind+1])-1);


        printf("opening ports %s, %s\n", port1name, port2name);

        if ((port1 = open(port1name, O_RDWR | O_NOCTTY))  == -1) {
            printf("-1 return opening %s: ", port1name);
            perror("com port open error: ");
            exit(-1);
        }
        if ((port2 = open(port2name, O_RDWR | O_NOCTTY))  == -1) {
            printf("-1 return opening %s: ", port2name);
            perror("com port open error: ");
            exit(-1);
        }

        while (1) {
            FD_ZERO(&rfds);
            FD_SET(port1, &rfds);
            FD_SET(port2, &rfds);

            // block until we have data
            rv = select(port2+1, &rfds, NULL, NULL, NULL); 
            if (rv < 0) {
                perror("Error in select:");
                exit(-1);
            }

            // get the command that woke us up & respond to it
            if (FD_ISSET(port1, &rfds)) {
                process_cmd(port1);
            }
            if (FD_ISSET(port1, &rfds)) {
                process_cmd(port1);
            }

        }
        return 0;
}

