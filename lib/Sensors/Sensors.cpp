#include "Sensors.h"

#define SD_CHIP 4
#define FILE_MAX_SIZE 4096

RTC_DS1307 rtc;
BME280I2C bme;
SdVolume sdCardSize;

extern Config config;

int code = 0;

/*
code 1 → Erreur accès GPS
code 2 → Erreur accès d'un capteur
code 3 → Données reçues d’un capteur incohérentes - vérification matérielle requise
code 4 → Erreur d’accès à l’horloge RTC
code 5 → Carte SD pleine
code 6 → Erreur d’accès ou d’écriture sur la carte SD

*/

Sensors sensors[]{
	{'L', "Light", 0, 0},
	{'T', "Temperature", 0, 0},
	{'H', "Hygrometry", 0, 0},
	{'P', "Pressure", 0, 0},
};

Sensor::Sensor()
{
}

void Sensor::initialize()
{

	rtc.begin();
	bme.begin();
}

float sensorTempValue(NAN), sensorHumValue(NAN), sensorPresValue(NAN);
int sensorLightValue = 0;

byte Sensor::setValues()
{
	BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
	BME280::PresUnit presUnit(BME280::PresUnit_Pa);
	bme.read(sensorPresValue, sensorTempValue, sensorHumValue, tempUnit, presUnit);
	int sensorLightValue = analogRead(LIGHT_PIN);

	bool sensorLightError = (sensorLightValue < config.getValue(F("LUMIN_LOW")) || sensorLightValue > config.getValue(F("LUMIN_HIGH"))) && config.getValue(F("LUMINO"));
	bool sensorTempError = (sensorTempValue < config.getValue(F("MIN_TEMP_AIR")) || sensorTempValue > config.getValue(F("MAX_TEMP_AIR"))) && config.getValue(F("TEMP_AIR"));
	bool sensorPresError = (sensorPresValue < config.getValue(F("PRESSURE_MIN")) || sensorPresValue > config.getValue(F("PRESSURE_MAX"))) && config.getValue(F("PRESSURE"));
	bool sensorHumError = (sensorTempValue < config.getValue(F("HYGR_MINT")) || sensorTempValue > config.getValue(F("HYGR_MAXT"))) && config.getValue(F("HYGR"));

	if (!rtc.begin())
	{
		code = 1;
	}
	else if (sensorTempError || sensorPresError)
	{
		code = 2;
	}
	else if (!bme.begin())
	{
		code = 3;
	}
	// else if (gpsError)
	// {
	// 	code = 4;
	// }
	// else if (SDWriteError)
	// {
	// 	code = 5;
	// }

	float value = 0;
	for (int i = 0; i < sizeof(sensors) / sizeof(Sensors); ++i)
	{

		switch (sensors[i].id)
		{
		case 'L':
			if (sensorLightError)
				sensors[i].error = true;
			else
			{
				sensors[i].error = false;
				value = sensorLightValue;
				sensors[i].value = value;
			}
			break;
		case 'T':
			if (sensorTempError || code == 2)
				sensors[i].error = true;
			else
			{
				sensors[i].error = false;
				value = sensorTempValue;
				sensors[i].value = value;
			}
			break;
		case 'H':
			if (sensorHumError)
				sensors[i].error = true;
			else
			{
				sensors[i].error = false;
				value = sensorHumValue;
				sensors[i].value = value;
			}
			break;
		case 'P':
			if (sensorPresError || code == 2)
				sensors[i].error = true;
			else
			{
				sensors[i].error = false;
				value = sensorPresValue;
				sensors[i].value = value;
			}
			break;
		}
	}

	return code;
}

File myFile;
DateTime now;

char filename[] = "yyyymmdd.txt";

String Sensor::changefileName()
{
	rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

	DateTime now = rtc.now();
	int year = now.year();
	int month = now.month();
	int day = now.day();

	filename[0] = (year / 1000) + '0';
	filename[1] = ((year % 1000) / 100) + '0';
	filename[2] = ((year % 100) / 10) + '0';
	filename[3] = (year % 10) + '0';

	filename[4] = (month / 10) + '0';
	filename[5] = (month % 10) + '0';

	filename[6] = (day / 10) + '0';
	filename[7] = (day % 10) + '0';
	String fName = String(filename);
	return fName;
}

uint32_t volumesize;

void Sensor::writetoSD(bool write)
{
	if (write)
	{
		volumesize = (sdCardSize.clusterCount() * sdCardSize.blocksPerCluster()) / 2;

		// if (volumesize > 4)
		// {
		// 	String oldfileName = filename;
		// 	String filename = changefileName();
		// }

		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		DateTime now = rtc.now();

		myFile = SD.open(filename, FILE_WRITE);

		if (myFile)
		{
			myFile.print(now.year(), DEC);
			myFile.print('-');
			myFile.print(now.month(), DEC);
			myFile.print('-');
			myFile.print(now.day(), DEC);
			myFile.print(' ');
			myFile.print(now.hour(), DEC);
			myFile.print(':');
			myFile.print(now.minute(), DEC);
			myFile.print(':');
			myFile.print(now.second(), DEC);

			for (int i = 0; i < sizeof(sensors) / sizeof(Sensors); ++i)
			{

				myFile.print(sensors[i].name);
				myFile.print(F("="));
				myFile.print(sensors[i].value);
				myFile.print(", "); // delimiter between data
			}

			// close the file:
			myFile.close();
			Serial.println(F("Done."));
		}
		else
		{
			// if the file didn't open, print an error:
			Serial.println(F("error opening DataSD.txt"));
		}
	}
}

void Sensor::showValues()

{

	rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	DateTime now = rtc.now();
	Serial.print(F("Date & Time: "));
	Serial.print(now.year(), DEC);
	Serial.print(F("/"));
	Serial.print(now.month(), DEC);
	Serial.print(F("/"));
	Serial.print(now.day(), DEC);
	Serial.print(now.hour(), DEC);
	Serial.print(F(":"));
	Serial.print(now.minute(), DEC);
	Serial.print(F(":"));
	Serial.println(now.second(), DEC);

	Serial.println(F("----------------"));
	for (int i = 0; i < sizeof(sensors) / sizeof(Sensors); ++i)
	{
		Serial.print(sensors[i].name);
		Serial.print(F(" : "));
		Serial.println(sensors[i].value);
	}

	Serial.println(F("------------------"));
}