
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
// #include "Sodaq_DS3231.h"
#include "../RTClib/RTClib.h"
//int code = 0

RTC_DS1307 rtc;

DateTime now;

int newHour = 0;
int oldHour = 0;


void write_data()
{
  File myFile;
  Wire.begin();
  rtc.begin();
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  if (!SD.begin(4))
  {
    Serial.println("initialization failed!");
    //code = 5;
    while (1)
      ;
  }
  Serial.println("initialization done.");
  now = rtc.now();
  oldHour = now.hour();

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile)
  {
    Serial.print("Writing to test.txt...");
    myFile.println("This is a test file :)");
    myFile.println("testing 1, 2, 3.");
    for (int i = 0; i < 20; i++)
    {
      myFile.println(i);
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void setup()
{
}

void loop()
{
  now = rtc.now();
  newHour = now.hour();
  if (oldHour != newHour)
  {
    
    oldHour = newHour;
  }
}