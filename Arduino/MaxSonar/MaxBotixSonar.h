/*
  SoftwareSerial.h - Software serial library
  Copyright (c) 2006 David A. Mellis.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/                              

#ifndef MaxBotixSonar_h
#define MaxBotixSonar_h

#include <inttypes.h>
#include "WConstants.h"

class MaxBotixSonar
{
  private:
    uint8_t _receivePin;
    long _baudRate;
    int _bitPeriod;
  public:
    MaxBotixSonar(uint8_t);
    void begin(long);
    int read();
};

#endif


