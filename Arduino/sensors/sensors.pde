
/*****************************************************************************
*     Copyright (C) 2009 Paul Bouchier                                       *
*                                                                            *
*     This program is free software: you can redistribute it and/or modify   *
*     it under the terms of the GNU General Public License as published by   *
*     the Free Software Foundation, either version 3 of the License, or      *
*     (at your option) any later version.                                    *
*                                                                            *
*     This program is distributed in the hope that it will be useful,        *
*     but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*     GNU General Public License for more details.                           *
*                                                                            *
*     You should have received a copy of the GNU General Public License      *
*     along with this program.  If not, see <http://www.gnu.org/licenses/>,  *
*     or the LICENSE-gpl-3 file in the root directory of this repository.    *
*                                                                            *
*****************************************************************************/
#include <Wire.h> 
//#include <TimerOne.h>
#include <Servo.h>
 
int compassAddress = 0x42 >> 1;  // From datasheet compass address is 0x42 
                                 // shift the address 1 bit right, the Wire library only needs the 7 
                                 // most significant bits for the address 

// Sonar pin assignments
int ss0EnableSonar = 8;
int ss0Rxpin = 9;
int ss1EnableSonar = 16;
int ss1Rxpin = 17;
int ss2EnableSonar = 2;
int ss2Rxpin = 3;

// Sonar serial parameters
long _baudRate;
int _bitPeriod;

// Servo
Servo sonar0Servo;
int servoPos = 90;  // variable to store the servo position 
  
int readCompass() 
{ 
  int reading = 0; 
  
  // step 1: instruct sensor to read echoes 
  Wire.beginTransmission(compassAddress); // transmit to device 
                              // the address specified in the datasheet is 66 (0x42) 
                              // but i2c adressing uses the high 7 bits so it's 33 
  Wire.send('A');             // command sensor to measure angle  
  Wire.endTransmission();     // stop transmitting 
 
  // step 2: wait for readings to happen 
  delay(10);                   // datasheet suggests at least 6000 microseconds 
  
  // step 3: request reading from sensor 
  Wire.requestFrom(compassAddress, 2);    // request 2 bytes from slave device #33 
 
  // step 4: receive reading from sensor 
  if(2 <= Wire.available())    // if two bytes were received 
  { 
    reading = Wire.receive();  // receive high byte (overwrites previous reading) 
    reading = reading << 8;    // shift high byte to be high 8 bits 
    reading += Wire.receive(); // receive low byte as lower 8 bits 
    reading /= 10; 
  } 
  return reading;
} 

/*
 * Read a byte from the sonar
 */
int MaxBotixSonarRead(int rxpin)
{
  int val = 0;
  //int bitDelay = _bitPeriod - clockCyclesToMicroseconds(50); // original delay
  //int bitDelay = _bitPeriod - clockCyclesToMicroseconds(250); // delay if using clkout
  int bitDelay = _bitPeriod - clockCyclesToMicroseconds(200); // delay if using timeout

  unsigned long timeoutAt;
  
  timeoutAt = millis() + 200;
  
  /* one byte of serial data (LSB first)
  **       /--\/--\/--\/--\/--\/--\/--\/--\/--\ 
  ** ...--/   /\--/\--/\--/\--/\--/\--/\--/\---\---....
  **	 start  0   1   2   3   4   5   6   7 stop
  */

  while (digitalRead(rxpin) == LOW)
  {
    if (millis() > timeoutAt) {
      Serial.println("Timeout");
      return -1;
    }
  }

  // confirm that this is a real start bit, not line noise
  if (digitalRead(rxpin) == HIGH) {
    // frame start indicated by a rising edge and high start bit
    // jump to the middle of the high start bit
    delayMicroseconds(bitDelay / 2 - clockCyclesToMicroseconds(50));
	
    // offset of the bit in the byte: from 0 (LSB) to 7 (MSB)
    for (int offset = 0; offset < 8; offset++) {
	// jump to middle of next bit
	delayMicroseconds(bitDelay);
	
        //digitalWrite(clkout, HIGH); // uncomment to set a clock out as each bit is received
        //digitalWrite(clkout, LOW);
	// read bit
	val |= (digitalRead(rxpin) ^ 1) << offset; // invert the read bit & or into rcv byte
    }
    delayMicroseconds(_bitPeriod);  // stop bit - really need to wait 1.5 bits    
    return val;
  }
  
  return -1;
}

char line[64];  // a buffer to hold the incoming line
char temp_line[80];

/*
 * blocking function to read serial input for command, waits until we've got max characters or we see a '\n'
 */
void usart_getline( char *line, int max)
{
    int length = 0;
    char temp = '\0';

    do
    {
      	if (Serial.available() > 0) {
	  temp = Serial.read(); // read the incoming byte:
          temp_line[length] = temp;    // store into buffer
          length++;
          if ( length == (max - 1) )  // we only have space for one more character in our buffer
            break;
      }
    } while ( (temp != '\n') && (temp != '\r') ); 

    temp_line[length] = '\0';
    strcpy( line, temp_line );

    return;
}

/* R U alive command XXX - response: COK */
void parse_XXX()
{
    // print response
    Serial.println( "COK");
}

/* Read Compass RCP command - response: 3 ascii digit compass reading */
void parse_RCP()
{
  int compass;
  char compass_string[4];
    
  compass = readCompass();
  compass_string[3] = '\0';
    
  for (int i=2; i>=0; i--) {
    compass_string[i] = compass%10 + 0x30;  // Print each digit as an ascii character
    compass /= 10;
  }
    
  // print response
  Serial.print( "DIR");
  Serial.println(compass_string);
}

void parse_PCP()
{
  while (Serial.available() == 0) {
    parse_RCP();
    delay(200);
  }
}

void parse_RS(char sensor_num)
{
  char c, d, e, f;
  int ssRxpin;
  int ssEnableSonar;
  
  // set the proper pin numbers for triggering & reading the sensor
  switch (sensor_num){
    case 0: ssRxpin = ss0Rxpin; ssEnableSonar = ss0EnableSonar; break;
    case 1: ssRxpin = ss1Rxpin; ssEnableSonar = ss1EnableSonar; break;
    case 2: ssRxpin = ss2Rxpin; ssEnableSonar = ss2EnableSonar; break;
    default: Serial.println("XXX"); return;
  }

  // trigger the sensor to take a reading, then delay while it acquires the data
  digitalWrite(ssEnableSonar, HIGH);
  delay(1);
  digitalWrite(ssEnableSonar, LOW);
  
  // read range back from sensor. 1st byte is 'R' - discard it
  c = MaxBotixSonarRead(ssRxpin);
  d = MaxBotixSonarRead(ssRxpin);
  e = MaxBotixSonarRead(ssRxpin);
  f = MaxBotixSonarRead(ssRxpin);
  
  if ((c==-1) || (d==-1) || (e==-1) || (f==-1)) {
    Serial.println("XXX"); // ERROR
    return;
  }
  Serial.print("RNG");
  Serial.print(d);
  Serial.print(e);
  Serial.print(f);
  Serial.println();

  return;
} 

/*
 * loop printing the specified sonar number
 */
void parse_PS (char sensor_num)
{
  while (Serial.available() == 0) {
    parse_RS (sensor_num);
    delay(200);
  }
}

/* 
 * Output a pwm pulse train on I/O 10
 */
void parse_SRV (char *servo_val)
{
  //int pwm_duty = 0;
  servoPos = 0;
  
  for (int i=0; i<=2; i++) {
    if (servo_val[i] < '0' || servo_val[i] > '9') {
      Serial.println("XXX");
      return;
    }
    //pwm_duty *= 10;
    //pwm_duty += pwm_val[i] - '0';
    servoPos *= 10;
    servoPos += servo_val[i] - '0';
  }
    //Timer1.setPwmDuty(10, pwm_duty);
    sonar0Servo.write(servoPos);
    Serial.println("SOK");
}

/*
 * Sweep the sonar back & forth
 */
void parse_SWP ()
{
  while (Serial.available() == 0) {
    for(servoPos = 0; servoPos < 180; servoPos += 1)  // goes from 0 degrees to 180 degrees 
    {                                  // in steps of 1 degree 
      sonar0Servo.write(servoPos);              // tell servo to go to position in variable 'pos' 
      delay(15);                       // waits 15ms for the servo to reach the position 
    } 
    for(servoPos = 180; servoPos>=1; servoPos-=1)     // goes from 180 degrees to 0 degrees 
    {                                
      sonar0Servo.write(servoPos);              // tell servo to go to position in variable 'pos' 
      delay(15);                       // waits 15ms for the servo to reach the position 
    } 
  }
}

void setup() {

  // initialize the serial port up to Linux
  Serial.begin(9600);
  // Serial.println("OK\n\r");    // signal startup
  
  // Compass setup
  Wire.begin();                // join i2c bus (address optional for master) 
  
  // sonar setup
  pinMode(ss0Rxpin, INPUT);
  pinMode(ss0EnableSonar, OUTPUT);
  digitalWrite(ss0EnableSonar, LOW);
  pinMode(ss1Rxpin, INPUT);
  pinMode(ss1EnableSonar, OUTPUT);
  digitalWrite(ss1EnableSonar, LOW);
  pinMode(ss2Rxpin, INPUT);
  pinMode(ss2EnableSonar, OUTPUT);
  digitalWrite(ss2EnableSonar, LOW);
  _baudRate = 9600;
  _bitPeriod = 1000000 / _baudRate;
  
  // Servo setup
  //Timer1.initialize(4000);    // 4ms period
  //Timer1.pwm(10, 512);        // 50% duty cycle
  sonar0Servo.attach(10);      // pin 10 is servo output
  sonar0Servo.write(servoPos);      // initial position 90 degrees
}

void loop () {
          // block for a '\n' terminated line, or 64 chars. which ever comes 1st
        usart_getline(line, 64);

        // debug
        //Serial.println( line);

        // compare the first three chars to known commands
        if( strncmp( line, "XXX", 3 ) == 0)         {
          parse_XXX();  // print COK
        }
        else if( strncmp( line, "RCP", 3 ) == 0 )         {
          parse_RCP();  // read compass & print value
        } 
        else if( strncmp( line, "PCP", 3 ) == 0 )         {
          parse_PCP();  // loop reading compass & printing value
        } 
        else if (strncmp( line, "RS", 2 ) == 0)         {
          if (line[2] >= '0' && line[2] <= '2')
            parse_RS(line[2] - '0');  // read designated sonar & print value
        } 
        else if (strncmp( line, "PS", 2 ) == 0) {
          if (line[2] >= '0' && line[2] <= '2')
            parse_PS(line[2] - '0');  // loop reading designated compass & printing value 
        }
        else if (strncmp( line, "SRV", 3 ) == 0) {
          parse_SRV(&line[3]);  // Pass degrees (0-180) value to servo routine
        }
        else if (strncmp( line, "SWP", 3 ) == 0) {
          parse_SWP();
        }
 }
