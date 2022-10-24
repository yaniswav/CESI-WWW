#include "../lib/Config/Config.h"
#include "../lib/LEDS/Led.h"
#include "../lib/Sensors/Sensors.h"
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

#define LIGHT_PIN 0

#define GREEN_BUTTON 3
#define RED_BUTTON 2

#define MODE_NORMAL 0
#define MODE_ECO 1
#define MODE_MAINTENANCE 2
#define MODE_CONFIG 3
#define MAX_VALUES 3
#define VERSION 1

Led leds(LED_PIN_1, LED_PIN_2, 1);

String batchNumber = "3W22_GR2";
Config config(VERSION, batchNumber);

#define chipSelect 4 // Lecteur carte SD

byte previousMode = MODE_NORMAL;
byte currentMode = MODE_NORMAL;

Sensor sens;

unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;

void redbuttonEvent()
{
  buttonPressedMs = millis();
  buttonPressed = true;
}

void greenbuttonEvent()
{
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
    break;
  case MODE_ECO:
    name = F("eco");
    break;
  case MODE_MAINTENANCE:
    name = F("maintenance");
    break;
  case MODE_CONFIG:
    name = F("configuration");
    break;
  }
  Serial.print(F("The new mode is: "));
  Serial.println(name); // Affiche le mode utilisé
}

void pressedButtonGreen() // Fonction si le bouton vert est pressé pendant 5 secondes
{
  if (currentMode == MODE_NORMAL)
    changeMode(MODE_ECO);
  else if (currentMode == MODE_ECO)
    changeMode(MODE_NORMAL);
}

void pressedButtonRed() // fonction si le bouton rouge est pressé pendant 5 secondes
{
  if (currentMode == MODE_MAINTENANCE || currentMode == MODE_CONFIG)
  {
    changeMode(previousMode);
    // while (!card.begin()) {
    //   Serial.println(F("SD Card loading Failed"));
    //   delay(5000);
    // }
  }
  else
  {
    previousMode = currentMode;
    changeMode(MODE_MAINTENANCE);
    // card.end();
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

bool checkError()
{

  Serial.println(F("Getting sensors values..."));
  byte errorCode = sens.setValues();
  sens.showValues();

  if (!errorCode)
  {
    return false;
  }

  switch (errorCode)
  {
  case 1:
    // erreur RTC
    leds.errorColor(F("RED"), 1, F("BLUE"), 1);
    break;
  case 2:
    // erreur données
    leds.errorColor(F("RED"), 1, F("GREEN"), 2);
    break;
  case 3:
    // erreur capteur
    leds.errorColor(F("RED"), 1, F("GREEN"), 1);
    break;
  case 4:
    // erreur GPS
    leds.errorColor(F("RED"), 1, F("YELLOW"), 1);
    break;
  case 5:
    // erreur d'écriture sur la carte SD
    leds.errorColor(F("RED"), 1, F("WHITE"), 2);
    break;
  case 6:
    // erreur carte SD pleine
    leds.errorColor(F("RED"), 1, F("WHITE"), 1);
    break;
  }
  return true;
}

void setup()
{
  Serial.begin(SERIAL_BAUD);

  leds.initialize();

  pinMode(GREEN_BUTTON, INPUT_PULLUP); // Initialisation bouton
  pinMode(RED_BUTTON, INPUT_PULLUP);   // Initialisation bouton

  attachInterrupt(digitalPinToInterrupt(RED_BUTTON), redbuttonEvent, FALLING);
  attachInterrupt(digitalPinToInterrupt(GREEN_BUTTON), greenbuttonEvent, FALLING);

  sens.initialize();
}

unsigned long lastSensorCheck(0);
int timeCheck;

void loop()
{
  checkButton(); // Vérifie le bouton et le temps de pression

  if (currentMode != MODE_CONFIG)
  {

    int timeCheck = config.getValue("LOG_INTERVAL"); // Changement des paramètres enregistrées

    if (currentMode == MODE_ECO)
    {
      timeCheck = timeCheck * 2; // Temps entre 2 mesures muliplié par 2
      leds.color(F("BLUE"));
    }

    if (currentMode == MODE_MAINTENANCE)
    {
      sens.showValues(); // Affichage des données
                         // sens.writetoSD(false);
      leds.color(F("ORANGE"));
    }

    leds.color(F("GREEN"));

    if ((millis() - lastSensorCheck) > (timeCheck * (60 * 1000)))
    {
      lastSensorCheck = millis();
      checkError();
      // sens.writetoSD(true); // Ecriture sur la carte SD
    }
  }

  else
  {
    leds.color(F("YELLOW"));

    unsigned long lastActivity = config.getlastActivity();
    if ((millis() - lastActivity) / 1000 > (30 * 60))
    {
      changeMode(MODE_NORMAL);
    }
    config.waitValues();
  }
}
