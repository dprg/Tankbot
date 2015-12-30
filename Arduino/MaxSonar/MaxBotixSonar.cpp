/*
  MaxBotixSonar.cpp - MaxBotix Software serial library
  Copyright (c) 2006 David A. Mellis.  All right reserved.
  Copyright (c) 2008 Paul Bouchier.  All right reserved.

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

/*
  This library is a reduced form of the SoftwareSerial library for Arduino, by
  David A. Mellis. It uses inverted logic, and has only a receive function and
  a way to start the sonar acquisition cycle. The Maxbotix sonar emits inverted
  serial (intended for applying to an inverting RS232 transmitter).
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "MaxBotixSonar.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

MaxBotixSonar::MaxBotixSonar(uint8_t receivePin)
{
  _receivePin = receivePin;
  pinMode(receivePin, INPUT);
  _baudRate = 0;
}

/******************************************************************************
 * User API
 ******************************************************************************/

void MaxBotixSonar::begin(long speed)
{
  _baudRate = 9600;
  _bitPeriod = 1000000 / _baudRate;
}

int MaxBotixSonar::read()
{
  int val = 0;
  int bitDelay = _bitPeriod - clockCyclesToMicroseconds(50);
  
  /* one byte of serial data (LSB first)
  **       /--\/--\/--\/--\/--\/--\/--\/--\/--\ 
  ** ...--/   /\--/\--/\--/\--/\--/\--/\--/\---\---....
  **	 start  0   1   2   3   4   5   6   7 stop
  */

  while (!digitalRead(_receivePin));

  // confirm that this is a real start bit, not line noise
  if (digitalRead(2) == HIGH) {
    // frame start indicated by a rising edge and high start bit
    // jump to the middle of the high start bit
    delayMicroseconds(bitDelay / 2 - clockCyclesToMicroseconds(50));
	
    // offset of the bit in the byte: from 0 (LSB) to 7 (MSB)
    for (int offset = 0; offset < 8; offset++) {
	// jump to middle of next bit
	delayMicroseconds(bitDelay);
	
	// read bit
	val |= (digitalRead(_receivePin) ^ 1) << offset; // invert the read bit & or into rcv byte
    }
	
    delayMicroseconds(_bitPeriod);	// stop bit
    return val;
  }
  return -1;
}

