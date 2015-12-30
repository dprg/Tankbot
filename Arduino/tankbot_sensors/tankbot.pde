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

int compass;
char line[64];  // a buffer to hold the incoming line
char temp_line[80];

// blocking function to wait until we've got max characters or we see a '\n'
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
void parse_XXX( char *line )
{
    // print response
    Serial.println( "COK");
}

/* Read Compass RCP command - response: 3 ascii digit compass reading */
void parse_RCP( char *line )
{
    char compass_string[4];
    int temp;
    
    temp = compass;
    compass_string[3] = '\0';
    
    for (int i=2; i>=0; i--) {
      compass_string[i] = temp%10 + 0x30;  // Print each digit as an ascii character
      temp /= 10;
    }
    
    compass += 10;
    if (compass > 359)
      compass = 0;
      
    // print response
    Serial.print( "COM");
    Serial.println(compass_string);
}

void setup() {

  Serial.begin(9600);
  Serial.println("OK\n\r");    // signal startup
  compass = 0;
  
}

void loop () {
          // block for a '\n' terminated line, or 64 chars. which ever comes 1st
        usart_getline(line, 64);

        // debug
        //Serial.println( line);

        // compare the first three chars to known commands
        if( strncmp( line, "XXX", 3 ) == 0)
        {
            parse_XXX( line );
        }
        else if( strncmp( line, "RCP", 3 ) == 0 )
        {
            parse_RCP( line );
        } 

 
}
