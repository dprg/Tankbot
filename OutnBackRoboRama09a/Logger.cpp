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

/*******************************************************************************
* This class is responsible for sending log messages to a file and to a socket *
*******************************************************************************/  


#include "Logger.h"

Logger::~Logger()
{
	logStream.flush();
	logStream.close();
}

int Logger::init(char *logfileName, int socketNum)
{
	logStream.open(logfileName, ios::out);
	return 0;
}

int Logger::log(char *msg)
{      
	logStream << msg << endl;
	return 0;
}
