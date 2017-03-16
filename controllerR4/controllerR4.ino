/* ROV remote controller arduino program    for eng100-600  by JAG, 10/2016
 * the data structure is 4 bytes plus a 5th check byte which
 * is the XOR product of the 4 data bytes.
 * in the first byte, bits 3,2,1,0 are motor aFor,aRec,bFor,bRev
 * in the second byte, bits 3,2,1,0 are motor cFor, cRev, dForm dRev
 * the third byte is the 1st servo position in degrees as the byte value
 * the fourth byte is the 2nd servo position..
 * 
 * to load the program, disconnect the BT xmitter to enable the USB connection to the programmer
 * 
 */


#define aFor 13                                //8 pins for 8 motor control functions
#define aRev 12                                // comment out the constant replacement 
#define bFor 11                                // for the motor pins if programming
#define bRev 10                                // a custom control 
#define cFor  9
#define cRev  8
#define dFor  7
#define dRev  6

#define servOnePin A0
#define servTwoPin A1
#define ledPin A5


const int setsPerSec = 6;                     //how fast to send data sets
const byte biteF = 0xFF;                      //this is the flag(start) byte
const byte PASS = 0x55;                       //confirmation byte
const byte FAIL = 0x0F;                       // Data check fail byte

byte chkbyte;
byte rdgs[5] = {0,0,0,0,0};                     //the data bytes read from the bluetooth
int pinMap[8] = {aFor, aRev, bFor, bRev, cFor, cRev, dFor, dRev};
int arrayOfst;                                 //+1 -1 holder for motor setting loop
int delayTime;


void setup()   {

  Serial.begin(115200);
  delay(1000);
  
  for ( int i = 0 ; i < 8 ; ++i )   {                //initialize dig pins to read inputs
      pinMode( pinMap[i], INPUT );
  }
  
  pinMode(servOnePin, INPUT);                         
  pinMode(servTwoPin, INPUT);
  pinMode( ledPin, OUTPUT);
  delayTime = 1000/setsPerSec;                         //calculate loop delay variable
}

void loop()  {
  
  if ( !Serial.available() ) {                       //read any bytes off the buffer
    digitalWrite ( ledPin, LOW );                    //if last byte is not a handshake turn off indicator
  }
  else {
    for ( int j = Serial.available() ; j >  0 ; --j ) {
      chkbyte = Serial.read();                       //read a byte. Is it the confirmation value?
    }
    if ( chkbyte == PASS ) {                         //if payload is getting data, turn on the indicator
      digitalWrite(ledPin, HIGH);
    }
    else  digitalWrite( ledPin, LOW);
  }
 
 
                                                    // write data to the xmit array
                                                    // the servo positions are the 3rd and 4th byte
                                                    
  rdgs[2] = byte(constrain(map(analogRead(servOnePin),0,1023,0,180),1, 179));
  rdgs[3] = byte(constrain(map(analogRead(servTwoPin),0,1023,0,180),1, 179)); 

     
/*****************DO NOT ALTER ABOVE THIS LINE !*************************************
 * The block between the comment lines is where the motor control logic can be changed
 * for custom control, comment it out and replace it with something else.
 * Your logic should require that if both the forward and reverse buttons are 
 * pressed at the same time, the controller sends "off" for both
 * to prevent any possibility of generating a shorted condition in the payload
 * 
 * The loop below that sets the output bits is an example of how you can
 * write very dense code in 'C'. This one loop is relatively arcane
 * to figure out but is short and easy to debug once you do. In the lines
 * further below is an example of the same operations done in single commands
 * to set each bit where things are more explicit.  
 */

  //commented out the for loop because we are customizing our buttons
  /*
  arrayOfst = 1;                                    // 1 or -1 picks the paired Foward or Reverse
  for ( int i = 0 ; i < 8 ; ++i )   {
      int k = i/4;                                  //this is either 0 or 1 to pick the byte
      int j = 3 - (i % 4);                          //this counts from 3 to 0 to pick the bit
      
      if ( digitalRead( pinMap[i] ) && !digitalRead(pinMap[i+arrayOfst] ) )  {
          bitWrite ( rdgs[k], j, 1 );               //only turn on if both bottons NOT pushed
      }
      else bitWrite( rdgs[k], j, 0 ) ;
      
      arrayOfst = arrayOfst * (-1);                 //flip the counter. For a "forward", the corresponding
                                                    //'Reverse' to check is the next value counting up. For a 'Reverse'
                                                    // it is the next value counting down.
  }*/
  
/**********************************************************************************/

/* in this block it's easier to see how to set each bit
 * based on some combination of switches. This can be used
 * as a starting point for designing a custom motor
 * switch control scheme.
 */

//For Reference: (action described by this code: associated bit to write)
//aFor: rdgs[0],3
//bFor: rdgs[0],1
//cFor: rdgs[1],3
//dFor: rdgs[1],1
//aRev: rdgs[0],2
//bRev: rdgs[0],0
//cRev: rdgs[1],2
//dRev: rdgs[1],0

//There are 4 thrusters on our ROV labeled here:
//a: up down
//b: forward back
//c: leftside angled
//d: rightside angled

  //ROV up (if thruster is forward and not reverse)
  //a thruster goes forward
  if ( digitalRead(aFor) && (!digitalRead(aRev)))  {                  //aFor
    bitWrite( rdgs[0], 3, 1);
  }
  else  bitWrite( rdgs[0], 3, 0);//returns off

  //ROV forward
  //b,c,d thruster goes forward
  if ( digitalRead(bFor) && (!digitalRead(bRev)))  {                  //bFor
    bitWrite( rdgs[0], 1, 1);
  }
  else  bitWrite( rdgs[0], 1, 0);

  //ROV right
  //d thruster reverse, c thruster partially forward [need to check this]
  if ( digitalRead(cFor) && (!digitalRead(cRev)))  {                  //cFor
    bitWrite( rdgs[1], 3, 1);
  }
  else  bitWrite( rdgs[1], 3, 0);
  
  //ROV left
  //c thruster reverse, d thruster partially forward [need to check this]
  if ( digitalRead(dFor) && (!digitalRead(dRev)))  {                  //dFor
    bitWrite( rdgs[1], 1, 1);
  }
  else  bitWrite( rdgs[1], 1, 0);
  
  //ROV down
  //a thruster reverse
  if ( digitalRead(aRev) && (!digitalRead(aFor)))  {                  //aRev
    bitWrite( rdgs[0], 2, 1);
  }
  else  bitWrite( rdgs[0], 2, 0);

  //ROV backward
  //b thruster reverse
  if ( digitalRead(bRev) && (!digitalRead(bFor)))  {                  //bRev
    bitWrite( rdgs[0], 0, 1);
  }
  else  bitWrite( rdgs[0], 0, 0);

  //ROV turn left
  //d thruster forward??
  if ( digitalRead(cRev) && (!digitalRead(cFor)))  {                  //cRev
    bitWrite( rdgs[1], 2, 1);
  }
  else  bitWrite( rdgs[1], 2, 0);

  //ROV turn right
  //c thruster forward??
  if ( digitalRead(dRev) && (!digitalRead(dFor)))  {                  //dRev
    bitWrite( rdgs[1], 0, 1);
  }
  else  bitWrite( rdgs[1], 0, 0);

 
/************************DO NOT ALTER BELOW THIS LINE**************/ 
  
  rdgs[4] = rdgs[0] ^ rdgs[1] ^ rdgs[2] ^ rdgs[3];                      //create check byte and send the start byte
  Serial.write(biteF);                                                  //and then the data array to the payload
  for (int i = 0 ; i < 5 ; ++i ) {
    Serial.write(rdgs[i]);
  }   
  delay(delayTime);                                                     //dont go faster than the payload can process data
}

