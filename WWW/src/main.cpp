#include "../lib/Config/Config.h"
#include "../lib/LEDS/Led.h"
#include "../lib/Sensors/Sensors.h"
#include <EnvironmentCalculations.h>

#include "../lib/RTClib/RTClib.h"

#include <EnvironmentCalculations.h>
#include <Wire.h>
#include <BME280.h>
#include <BME280I2C.h>
#include <BME280I2C_BRZO.h>
#include <BME280Spi.h>
#include <BME280SpiSw.h>


#define SERIAL_BAUD 9600
#define NUM_LEDS 1
#define LED_PIN_1 7
#define LED_PIN_2 8

#define GPS_PIN_1 4
#define GPS_PIN_2 5
#define LIGHT_PIN 0

#define GREEN_BUTTON 3
#define RED_BUTTON 2

#define MODE_NORMAL 0
#define MODE_ECO 1
#define MODE_MAINTENANCE 2
#define MODE_CONFIG 3
#define MAX_VALUES 3
#define VERSION 1

// include <SPI.h>
//#include <SD.h>
#include <SoftwareSerial.h>
#include <math.h>    // pour capteur luminosité
#include <TinyGPS.h> // pour GPS

Led leds(LED_PIN_1, LED_PIN_2, 1);

extern BME280I2C bme;
extern RTC_DS1307 rtc;

TinyGPS gps;
float lat, lon;
bool newdata = false;
// bool gpsEco = false;
// bool gpsNormal = false;

String batchNumber = "3W22_GR2";
Config config(VERSION, batchNumber);

#define chipSelect 4         // Lecteur carte SD
SoftwareSerial gpsloc(3, 4); // Rajouter le numéro de la PIN pour le GPS

byte previousMode = MODE_NORMAL;
byte currentMode = MODE_NORMAL;

Sensor sens;

unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;

bool SDWriteError = false;
bool gpsEco = false;
bool gpsError = false;

void redbuttonEvent()
{
  Serial.println("Pression rouge");
  buttonPressedMs = millis();
  buttonPressed = true;
}

void greenbuttonEvent()
{
  Serial.println("Pression vert");
  buttonPressedMs = millis();
  buttonPressed = true;
}

void changeMode(int mode_name)
{

  String name = F("");
  currentMode = mode_name;

  switch (mode_name)
  {
  case MODE_NORMAL:
    name = F("normal");
    Serial.println("GREEN");
    leds.color(F("GREEN"));
    // Standard();
    break;
  case MODE_ECO:
    name = F("eco");
    Serial.println("BLUE");
    leds.color(F("BLUE"));
    // Economique();
    break;
  case MODE_MAINTENANCE:
    name = F("maintenance");
    Serial.println("ORANGE");
    leds.color(F("ORANGE"));
    // Maintenance();
    break;
  case MODE_CONFIG:
    name = F("configuration");
    Serial.println("YELLOW");
    leds.color(F("YELLOW"));
    Configuration();
    break;
  }
  Serial.print(F("The new mode is: "));
  Serial.println(name);
}

void pressedButtonGreen()
{
  if (currentMode == MODE_NORMAL)
    changeMode(MODE_ECO);
}

void pressedButtonRed()
{
  if (currentMode == MODE_MAINTENANCE || currentMode == MODE_ECO)
  {
    changeMode(previousMode);
  }
  else
  {
    previousMode = currentMode;
    changeMode(MODE_MAINTENANCE);
  }
}

void checkButton()
{
  // Serial.println(buttonPressed);

  // Serial.println(millis() - buttonPressedMs);

  if (checkStartPressedButton && digitalRead(RED_BUTTON) == 0)
  {
    changeMode(MODE_CONFIG);
  }
  checkStartPressedButton = false;

  if ((millis() - buttonPressedMs) > (2 * 1000) && buttonPressed)
    Serial.println("check button time");
  {
    if (digitalRead(RED_BUTTON) == 0)
    {
      pressedButtonRed();
    }
    else if (digitalRead(GREEN_BUTTON) == 0)
    {
      pressedButtonGreen();
    }
    buttonPressed = false;
  }
}

bool checkError()
{

  Serial.println("Getting sensors values...");
  byte errorCode = sens.setValues();
  if (!errorCode)
  {
    return false;
  }

  switch (errorCode)
  {
  case 1:
    // rtc error
    leds.errorColor(F("RED"), 1, F("BLUE"), 1);
    break;
  case 2:
    // data error
    leds.errorColor(F("RED"), 1, F("GREEN"), 2);
    break;
  case 3:
    // sensor error
    leds.errorColor(F("RED"), 1, F("GREEN"), 1);
    break;
  case 4:
    // gps error
    leds.errorColor(F("RED"), 1, F("YELLOW"), 1);
    break;
  case 5:
    // SD card write error
    leds.errorColor(F("RED"), 1, F("WHITE"), 2);
    break;
  case 6:
    // SD card full
    leds.errorColor(F("RED"), 1, F("WHITE"), 1);
    break;
  }

  return true;
}

void setup()
{
  Serial.begin(SERIAL_BAUD);

  leds.initialize();
  leds.color("BLUE");

  pinMode(GREEN_BUTTON, INPUT_PULLUP); // Initialisation bouton
  pinMode(RED_BUTTON, INPUT_PULLUP);   // Initialisation bouton

  attachInterrupt(digitalPinToInterrupt(RED_BUTTON), redbuttonEvent, FALLING);
  attachInterrupt(digitalPinToInterrupt(GREEN_BUTTON), greenbuttonEvent, FALLING);
  sens.initialize();
}

unsigned long lastSensorCheck(0);

void loop()
{
  checkButton();

  // if (currentMode != MODE_CONFIG)
  // {

  //   if ((millis() - lastSensorCheck) > (10 * 1000))
  //   {
  //     lastSensorCheck = millis();
  //     checkError();
  //   }
  // }
  // else
  // {
  //   leds.color(F("YELLOW"));
  //   config.waitValues();
  // }
}
