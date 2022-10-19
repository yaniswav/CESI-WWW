#include "../lib/Config/Config.h"
#include "../lib/LEDS/Led.h"
#include <BME280.h>
#include <BME280I2C.h>
#include <BME280I2C_BRZO.h>
#include <BME280Spi.h>
#include <BME280SpiSw.h>
#include <EnvironmentCalculations.h>
#include <Wire.h>

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
#define NUM_VALUES 3
#define VERSION 1

// #include "DS1307.h"
// #include <SD.h>
// #include <SoftwareSerial.h>
// #include <math.h> // pour capteur luminosité
// #include <TinyGPS++.h> // pour GPS

BME280I2C bme;
RTC_DS1307 rtc;
// RTClib RTC;
// Tinygps gps;
// float lat, lon;
// bool newdata = false;
// bool gpsEco = false;
// bool gpsNormal = false;

String batchNumber = "3W22_GR2";
Config config(VERSION, batchNumber);

// #define chipSelect 4 // Lecteur carte SD
// SoftwareSerial gpsloc(...,...); // Rajouter le numéro de la PIN pour le GPS

Led leds(LED_PIN_1, LED_PIN_2, 1);

byte previousMode = MODE_NORMAL;
byte currentMode = MODE_NORMAL;

unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;

struct Sensor
{
  char id;
  String name;
  bool error;
  float average;
  float values[NUM_VALUES];
};

bool SDWriteError = false;
bool gpsEco = false;
bool gpsError = false;

Sensor sensors[]{
    {'L', "Light (Lumen)", 0, 0, {}},
    {'T', "Temperature (°C)", 0, 0, {}},
    {'H', "Hygrometry (%)", 0, 0, {}},
    {'P', "Pressure (HPa)", 0, 0, {}},
    {'W', "Water Temp (°C)", 1, 0, {}},
    {'C', "Water Current (Nœuds)", 1, 0, {}},
    {'F', "Wind Force (Nœuds)", 1, 0, {}},
    {'F', "Fine Particles (%)", 1, 0, {}}

};
/*
byte getSensorValues()

{

  int errorCode = 0;
  float sensorTempValue(NAN), sensorHumValue(NAN), sensorPresValue(NAN);
  int sensorLightValue = analogRead(LIGHT_PIN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, tempUnit, presUnit);

  //   bool sensorLightError = (sensorLightValue < config.getValue(F("LUMIN_LOW")) || sensorLightValue > config.getValue(F("LUMIN_HIGH"))) && config.getValue(F("LUMINO"));
  //   bool sensorTempError = (sensorTempValue < config.getValue(F("MIN_TEMP_AIR")) || sensorTempValue > config.getValue(F("MAX_TEMP_AIR"))) && config.getValue(F("TEMP_AIR"));
  //   bool sensorPresError = (sensorPresValue < config.getValue(F("PRESSURE_MIN")) || sensorPresValue > config.getValue(F("PRESSURE_MAX"))) && config.getValue(F("PRESSURE"));
  //   bool sensorHumError = (sensorTempValue < config.getValue(F("HYGR_MINT")) || sensorTempValue > config.getValue(F("HYGR_MAXT"))) && config.getValue(F("HYGR"));

  /*

    if (mode==eco)
  {
    gpseco=true;
    else(mode==normal)
    gpsNormal=true
  }

    while (gps.available()>0)
    {
      gps.encode(gpsloc.read());

      if (gps.location.isUpdated())
        {
      Serial.print("\t\tLAT="); Serial.print(gps.location.lat(), 6);
      Serial.print("LNG="); Serial.println(gps.location.lng(), 6);
        }
    }

  Serial.print("\t\tTemps: "); // Pour horloge
  Serial.print("a");
  Serial.print("\t\tSensorLight: ");
  Serial.print(sensorLightValue);
  Serial.println("lux");
  Serial.print("\t\ Temp: ");
  Serial.print(sensorTempValue);
  Serial.print("°" + String(tempUnit == BME280::TempUnit_Celsius ? 'C' : 'F'));
  Serial.print("\t\tHumidity: ");
  Serial.print(sensorHumValue);
  Serial.print("% RH");
  Serial.print("\t\tPressure: ");
  Serial.print(sensorPresValue);
  Serial.println("Pa");


      // }

       if (!rtc.begin()) {
          code = 1;
        }
        else if (sensorLightError || sensorTempError || sensorPresError || sensorHumError) {
          code = 2;
        }
        else if (!bme.begin())
        {
          code = 3;
        }
        else if (gpsError)
        {
          code = 4;
        }
        else if (SDWriteError) {
          code = 5;
        }
      */

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
    break;
  case MODE_ECO:
    name = F("eco");
    Serial.println("BLUE");
    leds.color(F("BLUE"));
    break;
  case MODE_MAINTENANCE:
    name = F("maintenance");
    Serial.println("ORANGE");
    leds.color(F("ORANGE"));
    break;
  case MODE_CONFIG:
    name = F("configuration");
    Serial.println("YELLOW");
    leds.color(F("YELLOW"));
    break;
  }
  Serial.print(F("The new mode is: "));
  Serial.println(name);
}

void pressedButtonGreen()
{
  if (currentMode == MODE_NORMAL)
    changeMode(MODE_ECO);
  else if (currentMode == MODE_ECO)
    changeMode(MODE_NORMAL);
}

void pressedButtonRed()
{
  if (currentMode == MODE_MAINTENANCE || currentMode == MODE_CONFIG)
  {
    changeMode(previousMode);
    // while (!card.begin())
    // {
    //   Serial.println(F("SD Card loading Failed"));
    //   delay(5000);
    // }
  }
  else
  {
    previousMode = currentMode;
    changeMode(MODE_MAINTENANCE);
    //card.end();
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

void setup()
{
  Serial.begin(SERIAL_BAUD);

  leds.initialize();
  leds.color("BLUE");

  pinMode(GREEN_BUTTON, INPUT_PULLUP); // Initialisation bouton
  pinMode(RED_BUTTON, INPUT_PULLUP);   // Initialisation bouton

  attachInterrupt(digitalPinToInterrupt(RED_BUTTON), redbuttonEvent, FALLING);
  attachInterrupt(digitalPinToInterrupt(GREEN_BUTTON), greenbuttonEvent, FALLING);
  // rtc.begin();

  // if (rtc.begin())
  // {
  //   DateTime now = rtc.now();
  //   Serial.print(now.day(), DEC);
  //   Serial.print(F("/"));
  //   Serial.print(now.month(), DEC);
  //   Serial.print(F("/"));
  //   Serial.print(now.year(), DEC);
  //   Serial.print(F(" "));
  //   Serial.print(now.hour(), DEC);
  //   Serial.print(F(":"));
  //   Serial.print(now.minute(), DEC);
  //   Serial.print(F(":"));
  //   Serial.println(now.second(), DEC);
  // }
  // else
  // {
  //   if (rtc.isrunning())
  //   {
  //     Serial.println(F("The RTC is stopped. Please run the setDate"));
  //   }
  //   else
  //   {
  //     Serial.println(F("RTC read error! Please check."));
  //   }
  // }

  // Wire.begin();

  // while (!bme.begin())
  // {
  //   Serial.println("Could not find BME280 sensor!");
  //   // delay(1000);
  // }

  // switch (bme.chipModel())
  // {
  // case BME280::ChipModel_BME280:
  //   Serial.println("Found BME280 sensor! Success.");
  //   break;
  // case BME280::ChipModel_BMP280:
  //   Serial.println("Found BMP280 sensor! No Humidity available.");
  //   break;
  // default:
  //   Serial.println("Found UNKNOWN sensor! Error!");
  // }
  config.waitValues();
}

void loop()
{

  checkButton();
  // getSensorValues();
  // DataloadSD();
}