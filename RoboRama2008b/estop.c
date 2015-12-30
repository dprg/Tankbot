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
#include <stdio.h>

int main( int argc, char *argv[] )
{
	H_tankbot tb;
	tb = tb_open("/dev/ttyS0");
	if(tb_estop(tb))
	{
		printf("Robot is talking.\n");
	}
	else
	{
		printf("Robot is not talking.\n");
	}
	tb_close(tb);
	return 0;
}

