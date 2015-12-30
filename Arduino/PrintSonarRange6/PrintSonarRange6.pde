    int ssEnableSonar = 8;
    int ssRxpin = 9;
    //int clkout = 10;
    long _baudRate;
    int _bitPeriod;

void setup()
{
  pinMode(ssRxpin, INPUT);
  pinMode(ssEnableSonar, OUTPUT);
  digitalWrite(ssEnableSonar, HIGH);
  //pinMode(clkout, OUTPUT);
  delay(1);
  digitalWrite(ssEnableSonar, LOW);
  _baudRate = 9600;
  _bitPeriod = 1000000 / _baudRate;

  // set the data rate for the SoftwareSerial port
  Serial.begin(9600);

}

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
    if (millis() > timeoutAt)
      return -1;
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


void loop(){
  char c, d, e, f;

  Serial.print("Sonar value: ");
  digitalWrite(ssEnableSonar, HIGH);
  delay(1);
  digitalWrite(ssEnableSonar, LOW);
  c = MaxBotixSonarRead(ssRxpin);
  d = MaxBotixSonarRead(ssRxpin);
  e = MaxBotixSonarRead(ssRxpin);
  f = MaxBotixSonarRead(ssRxpin);
  
  if ((c==-1) || (d==-1) || (e==-1) || (f==-1)) {
    Serial.println("Timeout on sonar read");
    delay(100);
    goto end;
  }
  Serial.print(c);
  Serial.print(d);
  Serial.print(e);
  Serial.print(f);
  Serial.println();

end:
delay (30);

} 
