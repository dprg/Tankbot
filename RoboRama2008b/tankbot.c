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
#include <sys/select.h>
#include <string.h>

int run_command();

// #define SIMULATION

H_tankbot tb_alloc()
{
	H_tankbot tb = 0;
	tb = malloc(sizeof(struct Tankbot));
	if(tb == NULL )
	{	
		perror( "tb_alloc:");
		return NULL;
	}
	
	// sane values
	tb->fd_rd=0;
	tb->fd_wr=0;

	tb->r_vel = tb->r_pos = tb->r_zero = 0;
	tb->l_vel = tb->l_pos = tb->l_zero = 0;

	return tb;
}

void tb_free( H_tankbot tb)
{
	free(tb);
}

H_tankbot tb_open( const char *fname )
{
	H_tankbot tb;
	tb=tb_alloc();
	if(tb == NULL)
		return NULL;
	
#ifdef SIMULATION
	fname = "/dev/null";
	printf("SIMULATION MODE ENABLED!!!\n");
#endif
	
	tb->fd_wr = open( fname, O_WRONLY | O_NOCTTY | O_NONBLOCK );
	if( tb->fd_wr == -1 )
	{
		perror("tb_open(fd_wr):");
		tb_free( tb );
		return NULL;
	}else
	{
		// set blocking mode (man 3 fcntl)
		fcntl( tb->fd_wr, F_SETFL, 0);
	}

	tb->fd_rd = open( fname, O_RDONLY | O_NOCTTY | O_NONBLOCK );
	if( tb->fd_rd == -1 )
	{
		perror("tb_open(fd_rd):");
		close(tb->fd_wr);
		tb_free(tb);
		return NULL;
	} else
	{
		//set blocking mode
		fcntl( tb->fd_rd, F_SETFL, 0);
	}
	return tb;
}

void tb_close( H_tankbot tb)
{
	close( tb->fd_wr );
	close( tb->fd_rd );
	tb_free(tb);
}

int tb_estop( H_tankbot tb)
{
	char input_buffer[10];
	int rv;

	assert(tb);

#ifdef SIMULATION
	return 1;
#endif

	rv = run_command(tb, "XXX\n\0", input_buffer, 9);
	if (rv > 0) {
		//printf("XXX output: %s", input_buffer);
	}
	if(rv != 4) return 0;
	return (strncmp(input_buffer, "COK", 3) == 0);
}
	
// fd_rd is file descriptor to read from (must be readable)
// input_buffer is pointer to char array( preallocated, data_to_read_length long at least)
// data_to_read_length is number of bytes to read
// timeout is seconds to wait for timeout
// returns number of bytes read into input_buffer or -1 on error
int read_with_timeout( int fd_rd, char *input_buffer, int data_to_read_length, float timeout)
{
	int io_length;
	
	fd_set rfds;
	struct timeval tv;
	int retval;

#ifdef SIMULATION
	return 0;
#endif

	io_length = 0;		// none yet
	while ( (io_length < data_to_read_length)
          ) 
	{
//		printf("-- %hhx\n", input_buffer[io_length-1]);
		// setup file descriptor set
		FD_ZERO(&rfds);
		FD_SET(fd_rd, &rfds);
		// setup our timeout
		tv.tv_sec = timeout;
		tv.tv_usec = (timeout - (int) timeout ) * 200000 ;

		// block until something happens (timeout or we have data)
		retval = select(fd_rd+1, &rfds, NULL, NULL, &tv); 

		if( retval == 1)
		{
			// we have data to read, so start grabbing
			//
			io_length += read(fd_rd, input_buffer+io_length, data_to_read_length-io_length);
			if(input_buffer[io_length-1]=='\n') break;
		} else if( retval == 0 )
		{
			// if 0, then no data to read (ie: timeout)
			fprintf(stderr, "read_with_timeout: read done timedout after %d bytes\n", io_length);
			return 0;	// we got something, but not as much as we wanted( probably)
		} else
		{
			// -1 is error
			// anything else is cosmic rays
			perror("read_with_timeout: ");
			return -1;
		}
	} // end of while filling input_buffer

	// everything was good, we found everything we were looking for
    input_buffer[io_length] = '\0';
    // printf("read with timeout returned %s\n", input_buffer);
	return io_length;
}

/*
 * This function sends a command (which is expected to have a terminating null when passed
 * as an input parameter) to the AVR board, and waits for a reply that matches the
 * command sent. Return of -1 indicates error reading output from AVR board.
 */
int run_command(H_tankbot tb, char *cmd, char *input_buffer, int data_to_read_length)
{
	int io_length;

	if (strlen(cmd) > 20) {
		printf("run_command passed too long command: %s\n", cmd);
		tb_estop(tb);
		exit(-1);
	}

	io_length = write( tb->fd_wr, cmd, strlen(cmd));

	if (io_length != strlen(cmd)) {
		printf("failed to write entire command %s\n", cmd);
		tb_estop(tb);
		exit(-1);
	}
reread:
	io_length = read_with_timeout( tb->fd_rd, input_buffer, data_to_read_length, TB_ESTOP_TIMEOUT);
	
	if (io_length == -1) {
		return -1;
	} else if (io_length == 0) {	// we might have read something, but it didn't end in \n
		return 0;
	} else {
		input_buffer[io_length] = 0;	// make a null-terminated string
		if (!strncmp(cmd, "XXX", 3)) {
			if (strncmp(input_buffer, "COK", 3)) {
				// printf("Error in XXX command, return was %s\n", input_buffer);
				return -1;		// expect COK back from XXX
			}
			return io_length;	// looks good
		}
		if (!strncmp(cmd, "RVL", 3)) {
			if (strncmp(input_buffer, "VEL", 3)) {
				// printf("Error in RVL command, return was %s\n", input_buffer);
				goto reread;
			}
			return io_length;	// looks good
		}
		if (!strncmp(cmd, "ROD", 3)) {
			if (strncmp(input_buffer, "ODO", 3)) {
				// printf("Error in ROD command, return was %s\n", input_buffer);
				goto reread;
			}
			return io_length;	// looks good
		}
		if (!strncmp(cmd, "LMT", 3)) {
			if (strncmp(input_buffer, "COK", 3)) {
				// printf("Error in LMT command, return was %s\n", input_buffer);
				goto reread;
			}
			return io_length;	// looks good
		}
		if (!strncmp(cmd, "RMT", 3)) {
			if (strncmp(input_buffer, "COK", 3)) {
				// printf("Error in RMT command, return was %s\n", input_buffer);
				goto reread;
			}
			return io_length;	// looks good
		}

		// Unchecked command - just return
		return io_length; 
	}
}

void tb_update_odometry( H_tankbot tb)
{
	char input_buffer[ 80 ];
	int rv;

	assert(tb);

#ifdef SIMULATION
	tb->l_pos += tb->l_vel;
	tb->r_pos += tb->r_vel;
	
	tb->l_pos -= tb->l_zero;
	tb->r_pos -= tb->r_zero;
	return;
#endif

	rv = run_command(tb, "ROD\n\0", input_buffer, 79);
	if (rv > 0) {
		// printf("ROD output: %s", input_buffer);
		sscanf( input_buffer, "ODO %ld %ld\n", &(tb->l_pos), &(tb->r_pos) );

		tb->r_pos -= tb->r_zero;
		tb->l_pos -= tb->l_zero;
		printf("lp   %ld ", tb->l_pos);
		printf("lp-z %ld ", tb->l_pos);
		printf("rp   %ld ", tb->r_pos);
		printf("rp-z %ld ", tb->r_pos);
	}

	rv = run_command(tb, "RVL\n\0", input_buffer, 79);
	if (rv > 0) {
		// printf("RVL output: %s", input_buffer);
		sscanf( input_buffer, "VEL %ld %ld\n", &(tb->l_vel), &(tb->r_vel) );
		printf("lv %ld rv %ld\n", tb->l_vel, tb->r_vel);
	}

}

void tb_reset_odometry( H_tankbot tb)
{
	// TODO for whatever reason, the first call always returns zero, so we call a bunch of times and it works. yay?
	tb_update_odometry( tb);
	tb_update_odometry( tb);
	tb_update_odometry( tb);
	tb->l_zero = tb->l_pos;
	tb->r_zero = tb->r_pos;
//	printf("rz   %d \n", tb->r_zero); 
	tb->l_pos=0;
	tb->r_pos=0;
}

void tb_set_motor( H_tankbot tb, int side, float power)
{
	signed int scaled_power = (signed char)(power * 127);
	char buffer[ 80 ];
	int rv;

	assert(tb);

	// enforce the input restrictions
	if( power > 1.0) scaled_power = 127;
	if( power < -1.0) scaled_power = -127;

	if(side & TB_RIGHT)
	{
#ifdef SIMULATION 
		// power = 1.0, vel= 1 inch per second
		tb->r_vel = power * TICKS_PER_INCH;
#endif
		sprintf(buffer, "LMT %d\n", scaled_power );
		rv = run_command(tb, buffer, buffer, 79);
	}

	if(side & TB_LEFT)
	{
#ifdef SIMULATION
		tb->l_vel = power * TICKS_PER_INCH;
#endif
		sprintf(buffer, "RMT %d\n", scaled_power );
		rv = run_command(tb, buffer, buffer, 79);
	}
}
