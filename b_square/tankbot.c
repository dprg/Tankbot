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
	int io_length = 0;
	char input_buffer[4];

	assert(tb);

#ifdef SIMULATION
	return 1;
#endif

	io_length = write( tb->fd_wr, "XXX\n", 4);
	//printf("wrote %d bytes\n", io_length);

	io_length = read_with_timeout( tb->fd_rd, input_buffer, 4, TB_ESTOP_TIMEOUT);
	if(io_length != 4) return 0;
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
		tv.tv_usec = (timeout - (int) timeout ) * 1000000 ;

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
			return io_length;	// we got something, but not as much as we wanted( probably)
		} else
		{
			// -1 is error
			// anything else is cosmic rays
			perror("read_with_timeout: ");
			return -1;
		}
	} // end of while filling input_buffer

	// everything was good, we found everything we were looking for
	return io_length;
}

void tb_update_odometry( H_tankbot tb)
{
	int io_length = 0;
	char input_buffer[ 80 ];

	assert(tb);

#ifdef SIMULATION
	tb->l_pos += tb->l_vel;
	tb->r_pos += tb->r_vel;
	
	tb->l_pos -= tb->l_zero;
	tb->r_pos -= tb->r_zero;
	return;
#endif

	io_length = write( tb->fd_wr, "ROD\n", 4);
	io_length = read_with_timeout( tb->fd_rd, input_buffer, 79, TB_ESTOP_TIMEOUT);
	input_buffer[io_length] = 0;	// make a string
	// TODO: if the scanf fails, we just borked the r_pos and l_pos
	sscanf( input_buffer, "ODO %ld %ld\n", &(tb->l_pos), &(tb->r_pos) );

	io_length = write( tb->fd_wr, "RVL\n", 4);
	io_length = read_with_timeout( tb->fd_rd, input_buffer, 79, TB_ESTOP_TIMEOUT);
	input_buffer[io_length] = 0;	// make a string
	// TODO: if the scanf fails, we just borked the r_vel and l_vel
	sscanf( input_buffer, "VEL %ld %ld\n", &(tb->l_vel), &(tb->r_vel) );

//	printf("rp   %d \n", tb->r_pos);
	tb->r_pos -= tb->r_zero;
	tb->l_pos -= tb->l_zero;
//	printf("rp-z %d \n", tb->r_pos);
}

void tb_reset_odometry( H_tankbot tb)
{
	// TODO for whatever reason, the first call always returns zero, so we call a bunch of times and it works. yay?
	tb_update_odometry( tb);
	tb_update_odometry( tb);
	tb_update_odometry( tb);
	tb->r_zero += tb->r_pos;
//	printf("rz   %d \n", tb->r_zero);

	tb->l_zero += tb->l_pos;
	tb->l_pos=0;
	tb->r_pos=0;
}

void tb_set_motor( H_tankbot tb, int side, float power)
{
	signed int scaled_power = (signed char)(power * 127);
	int io_length = 0;
	char buffer[ 80 ];

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
		io_length = write( tb->fd_wr, buffer, strlen(buffer));
		io_length = read_with_timeout( tb->fd_rd, buffer, 79, TB_ESTOP_TIMEOUT);
		// TODO: we should check the result
	}

	if(side & TB_LEFT)
	{
#ifdef SIMULATION
		tb->l_vel = power * TICKS_PER_INCH;
#endif
		sprintf(buffer, "RMT %d\n", scaled_power );
		io_length = write( tb->fd_wr, buffer, strlen(buffer));
		io_length = read_with_timeout( tb->fd_rd, buffer, 79, TB_ESTOP_TIMEOUT);
		// TODO: we shoudl check result
	}
}
