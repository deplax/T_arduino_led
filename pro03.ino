#include <mthread.h>

//Pin connected to latch pin (ST_CP) of 74HC595
const int latchPin = 3;
//Pin connected to clock pin (SH_CP) of 74HC595
const int clockPin = 4;
////Pin connected to Data in (DS) of 74HC595
const int dataPin = 2;

const int sensorPin = 0;
int sensorValue;
int state = 0;
int startTime = 0;
int finishTime = 0;

int markerS = 0;
int markerF = 0;

int lapTime = 0;

int toggle = 0;

int passFlag = 0;

const int slice = 100;

byte dataArray01[slice] = 
{
   0xFF , 0xE1 , 0xC0 , 0x9E , 0x9E , 0x9E , 0xC0 , 0xC0 
 , 0xFF , 0xFF , 0x80 , 0x80 , 0xF3 , 0xF3 , 0xFF , 0xFF 
 , 0x80 , 0x80 , 0xFE , 0xFE , 0xFE , 0xC8 , 0xC8 , 0xC9 
 , 0x80 , 0x80 , 0xFF , 0xFF , 0xFF , 0xF3 , 0x93 , 0x92 
 , 0x92 , 0x92 , 0x92 , 0xF3 , 0xF3 , 0xFF , 0x80 , 0x80 
 , 0xFC , 0xFC , 0xFF , 0xFF , 0xFE , 0xC0 , 0xC0 , 0xFF 
 , 0xF9 , 0xF9 , 0x80 , 0x80 , 0xFF , 0x80 , 0x80 , 0xFF 
 , 0xFF , 0xFF , 0xF1 , 0xE0 , 0xC4 , 0xCE , 0xCE , 0xCE 
 , 0xCE , 0xC4 , 0xE0 , 0xF1 , 0xFF , 0xFF , 0xFF , 0xFF 
 , 0xE0 , 0xE0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xE0 , 0xE0 
 , 0xFF , 0xFF , 0xFF , 0xFF , 0xE0 , 0xE0 , 0xFF , 0xFF 
 , 0xFF , 0xFF , 0xE0 , 0xE0 , 0xFF , 0xFF , 0xFF , 0xFF 
 , 0xE0 , 0xE0 , 0xFF , 0xFF 
};
byte dataArray02[slice] = 
{
   0xFF , 0xFF , 0xFF , 0x41 , 0x41 , 0x79 , 0x79 , 0xF9 
 , 0xF9 , 0xF9 , 0x19 , 0x19 , 0xFF , 0xFF , 0xFF , 0xFF 
 , 0x7F , 0x77 , 0x63 , 0x41 , 0x49 , 0x49 , 0x49 , 0xC9 
 , 0x41 , 0x63 , 0xF7 , 0xFF , 0xFF , 0xFF , 0x1F , 0x0F 
 , 0x4F , 0x4F , 0x0F , 0x1F , 0xFF , 0xFF , 0x01 , 0x01 
 , 0xFF , 0xFF , 0xDF , 0x8F , 0x1F , 0x3F , 0x3F , 0x1F 
 , 0x8F , 0xDF , 0x01 , 0x01 , 0xFF , 0x01 , 0x01 , 0xFF 
 , 0xE7 , 0xE7 , 0xE7 , 0x87 , 0x07 , 0x67 , 0x67 , 0x67 
 , 0x67 , 0x07 , 0x87 , 0xE7 , 0xE7 , 0xE7 , 0xFF , 0xFF 
 , 0x13 , 0x13 , 0xFF , 0xFF , 0xFF , 0xFF , 0x13 , 0x13 
 , 0xFF , 0xFF , 0xFF , 0xFF , 0x13 , 0x13 , 0xFF , 0xFF 
 , 0xFF , 0xFF , 0x13 , 0x13 , 0xFF , 0xFF , 0xFF , 0xFF 
 , 0x13 , 0x13 , 0xFF , 0xFF 
};



class checkSensor : public Thread
{
  public:
    checkSensor(int id);
  protected:
    bool loop();
  private:
    int id;
};

checkSensor::checkSensor(int id)
{
    this->id = id;
}

bool checkSensor::loop()
{
  
  int p = isPass();
  Serial.println(passFlag);
  if(p == 1)
    passFlag = 1;
  
  return true;
}

class printLED : public Thread
{
  public:
    printLED(int id);
  protected:
    bool loop();
  private:
    int id;
};

printLED::printLED(int id)
{
    this->id = id;
}

bool printLED::loop()
{
  // Die if requested:
  if(kill_flag)
    return false;
 
  if(passFlag == 1){
    passFlag = 0;
    lapTime = getLapTime();
    Serial.println(lapTime);
    float t = (float)lapTime/(float)slice;
    
    int i = 0;
    for(int i = 0; i < slice; i++)
    {
      //Serial.println(passFlag);
      if(passFlag == 1){
        Serial.println("???");
        break;
      }
      digitalWrite(latchPin, 0);
      shiftOut(dataPin, clockPin, dataArray01[i]);
      shiftOut(dataPin, clockPin, dataArray02[i]);
      digitalWrite(latchPin, 1);
      
      markerS = millis();
      int markerFlag = 1;
      while(markerFlag)
      {
        
        markerF = millis();
        //Serial.println(markerF - markerS);
        if((markerF - markerS) > (t - 0.5)){
          if(isPass() == 1){
            passFlag = 1;
          }
          markerFlag = 0;
        }
      }
      if(isPass() == 1){
        passFlag = 1;
      }
    }
 
  }
  return true;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin, OUTPUT);
  Serial.begin(9600);
  
  main_thread_list->add_thread(new checkSensor(0));
  main_thread_list->add_thread(new printLED(1));
  
}

//void loop() {
  // put your main code here, to run repeatedly:
  
//  if(isPass() == 1){
//    lapTime = getLapTime();
//    Serial.println(lapTime);
//    
//    int i = 0;
//    for(int i = 0; i < 80; i++)
//    {
//      shiftOut(dataPin, clockPin, dataArray01[i]);
//      shiftOut(dataPin, clockPin, dataArray02[i]);
//    }
//    
//  }




//  

//  int p = isPass();
//  Serial.println(p);
//  
//  digitalWrite(latchPin, 0);
//  if(p == 1){
//    if(toggle == 0){
//      toggle = 1;
//      shiftOut(dataPin, clockPin, dataArray01[0]);   
//      shiftOut(dataPin, clockPin, dataArray02[0]);
//    }else{
//      toggle = 0;
//      shiftOut(dataPin, clockPin, dataArray01[1]);   
//      shiftOut(dataPin, clockPin, dataArray02[1]);
//    }
//  }
//  digitalWrite(latchPin, 1);
  
//  delay(15);
//}

int isPass(){
  sensorValue = analogRead(sensorPin);
  //Serial.println(sensorValue);
  if(sensorValue > 512 && state == 0){
    state = 1;
    return 1;
  }
  else if(sensorValue <= 512 && state == 1){
    state = 0;
  }
  return 0;
}

int getLapTime(){
  int t = 0;
  finishTime = millis();
//  Serial.println(startTime);
//  Serial.println(finishTime);
  t = finishTime - startTime;
  startTime = millis();
  return t;
}


void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}
