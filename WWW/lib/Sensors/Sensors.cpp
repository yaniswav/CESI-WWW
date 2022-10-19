#include "Sensors.h"

RTC_DS1307 rtc;
BME280I2C bme;

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

	Serial.println("Initialisation des capteurs...");

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

	// bool sensorTempError, sensorLightError, sensorPresError, sensorHumError = false;

	// Serial.print("\t\tTemps: "); // Pour horloge
	// Serial.print("\t\tSensorLight: ");
	// Serial.print(sensorLightValue);
	// Serial.println("lux");
	// Serial.print("\t\ Temp: ");
	// Serial.print(sensorTempValue);
	// Serial.print("°" + String(tempUnit == BME280::TempUnit_Celsius ? 'C' : 'F'));
	// Serial.print("\t\tHumidity: ");
	// Serial.print(sensorHumValue);
	// Serial.print("% RH");
	// Serial.print("\t\tPressure: ");
	// Serial.print(sensorPresValue);
	// Serial.println("Pa");

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
	code = 3;
	Serial.println(code);
	return code;
}

void Sensor::showValues()

{
	setValues();
	Serial.println(F("----------------"));
	for (int i = 0; i < sizeof(sensors) / sizeof(Sensors); ++i)
	{
		Serial.print(sensors[i].name);
		Serial.print(" : ");
		Serial.println(sensors[i].value);
	}

	Serial.println(F("------------------"));
}
