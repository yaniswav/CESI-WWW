/
#include <SPI.h>
#include <SD.h>
#include "DS3231.h"


RTClib RTC;
DS3231 Clock;

int Year;
int Month;
int Date;
int Hour;
int Minute;
int Second;
int tempC;
int interval;
int Minute_last;

File myFile;

void setup() {
  Serial.begin(9600);
  Wire.begin(); //pour le DS3231
  while (!Serial) {
    ; // attendre le serial port d'etre brancher. Besoin pour les connections avec un port USB seulement.
  delay(2000);  //Après le Reset de la carte arduino, attendre 2 seconde pour enlever la Carte SD

  Serial.print("Connection de la carte SD");
  if (!SD.begin(10)) {
    Serial.println("Echec de la connection à carte SD !!");
  }
  else
  {
    Serial.println("Connection de la carte SD ok.");
  }
}


void loop() {
  interval = 1; //interval pour écrire les données
  DateTime now = RTC.now();
  Year = now.year();
  Month = now.month();
  Date = now.day();
  Hour = now.hour();
  Minute = now.minute();
  Second = now.second();
  tempC = Clock.getTemperature();
  
  if ((Minute % interval == 0)&(Minute_last!=Minute))
  {
    write_data(tempC);  //Ecriture des données
    Minute_last = Minute;
  }
}

void write_data(int temperature_input)
{
  myFile = SD.open("DataSD.txt", FILE_WRITE);

  //Si le dossiers est ok, écriture des données.
  if (myFile) {
    myFile.print(Year);
    myFile.print("/");
    myFile.print(Month);
    myFile.print("/");
    myFile.print(Date);
    myFile.print(" ");
    myFile.print(Hour);
    myFile.print(":");
    myFile.print(Minute);
    myFile.print("_temperature_");
    myFile.println(sensorTempValue);
    myFile.print("_Humidity_");
    myFile.println(sensorHumValue);
    myFile.print("_% RH_");
    myFile.print("_Pressure:_");
    myFile.println(sensorPresValue);
    myFile.print("_Pa_");
    myFile.print("_Luminosite_");
    myFile.println(sensorLightValue);
    myFile.print("_Lux_");

    myFile.close();
    Serial.println("Ecriture du dossier ok."); //Ecrire la sortie du Port COM.
  } else {
    Serial.println("erreur de l'ouverture du dossier DataSD");
  }
}
