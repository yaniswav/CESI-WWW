#include "Config.h"

RTC_DS1307 time;
long lastActivity = 0;

Configuration conf[] = {
	{0, "VERSION", 0, 20, true, 1, 1},
	{1, "LUMINO", 0, 1, true, 1, 2},
	{255, "LUMIN_LOW", 0, 1023, false, 2, 3},
	{768, "LUMIN_HIGH", 0, 1023, false, 2, 5},
	{1, "TEMP_AIR", 0, 1, true, 1, 6},
	{-10, "MIN_TEMP_AIR", -40, 85, true, 2, 7},
	{60, "MAX_TEMP_AIR", -40, 85, true, 2, 8},
	{1, "HYGR", 0, 1, true, 1, 9},
	{0, "HYGR_MINT", -40, 85, true, 2, 10},
	{50, "HYGR_MAXT", -40, 85, true, 2, 11},
	{1, "PRESSURE", 0, 1, true, 1, 12},
	{850, "PRESSURE_MIN", 300, 1100, false, 2, 13},
	{1080, "PRESSURE_MAX", 300, 1080, false, 2, 15},
	{10, "LOG_INTERVAL", 1, 255, false, 2, 17},
	{4096, "FILE_MAX_SIZE", 1024, 8192, false, 2, 19},
	{30, "TIMEOUT", 1, 255, true, 1, 21}};

Config::Config(byte version, String batchNumber) : _batchNumber(batchNumber)
{
}

void Config::resetValues()
{
	int i = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i)
	{
		writeValues(i, conf[i].value);
	}
}

void Config::getVersion()
{
	Serial.print(F("The version of program is : "));
	Serial.println(conf[getIndex("VERSION")].value);
	Serial.print(F("The batch number is : "));
	Serial.println(_batchNumber);
}

byte Config::getIndex(String name)
{
	byte result = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i)
	{
		result = i;
		if (conf[i].name == name)
			break;
	}
	return result;
}

byte Config::getSize(int index)
{
	byte result = 0;
	result = conf[index].size;
	return result;
}

int Config::getValue(String name)
{
	int index = getIndex(name);
	int value;
	value = read(index, getSize(index));
	return value;
}

int Config::read(int index, int _size)
{
	int value = 0;

	for (int i = 0; i < _size; i++)
	{
		value |= EEPROM.read(index + i) << (8 * i);
	}
	return value;
}

void Config::setValue(String name, int newValue)
{
	int index = getIndex(name);
	writeValues(index, newValue);
}

void Config::writeValues(int index, int value)
{

	if (conf[index].isByte == true)
	{
		EEPROM.update(conf[index].eepromAdr, value);
	}

	else
	{
		EEPROM.put(conf[index].eepromAdr, value);
	}
}

void Config::showValues()
{
	Serial.println(F("------------------"));
	getVersion();
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i)
	{
		String name = conf[i].name;
		Serial.print(name);
		Serial.print(F(" : "));
		Serial.println(read(i, conf[i].size));
	}

	Serial.println(F("------------------"));
}

long Config::getlastActivity()
{
	return lastActivity;
}

String splitString(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = {0, -1};
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++)
	{
		if (data.charAt(i) == separator || i == maxIndex)
		{
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void Config::waitValues()
{
	String data;
	String name;
	int newValue;
	if (Serial.available() > 0)
	{
		lastActivity = millis();
		data = Serial.readString();
		name = splitString(data, '=', 0);
		name.trim();
		name.toUpperCase();
		if (name == "SHOW" || name == "RESET" || name == "VERSION")
		{

			if (name == "RESET")
			{
				resetValues();
			}
			else if (name == "VERSION")
			{
				getVersion();
				return;
			}
			showValues();
			return;
		}
		if (name == "CLOCK" || name == "DATE" || name == "DAY")
		{
			DateTime now = time.now();
			if (name == "CLOCK")
			{
				String clock = splitString(data, '=', 1);
				int hour = splitString(clock, ':', 0).toInt();
				int minute = splitString(clock, ':', 1).toInt();
				int second = splitString(clock, ':', 2).toInt();
				if (hour > 23 || hour < 0 || minute > 59 || minute < 0 || second > 59 || second < 0)
				{
					Serial.println(F("The correct format is : HOUR{0-23}:MINUTE{0-59}:SECOND{0-59}"));
					return;
				}
				time.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, second));
			}
			else if (name == "DATE")
			{
				String date = splitString(data, '=', 1);
				int month = splitString(date, ',', 0).toInt();
				int day = splitString(date, ',', 1).toInt();
				int year = splitString(date, ',', 2).toInt();
				if (month > 12 || month < 1 || day > 31 || day < 1 || year > 2099 || year < 2000)
				{
					Serial.println(F("The correct format is : MONTH{1-12},DAY{1-31},YEAR{2000-2099}"));
					return;
				}
				time.adjust(DateTime(year, month, day, now.hour(), now.minute(), now.second()));
			}

			Serial.print(F("The new date is : "));
			now = time.now();
			Serial.print(now.day(), DEC);
			Serial.print(F("/"));
			Serial.print(now.month(), DEC);
			Serial.print(F("/"));
			Serial.print(now.year(), DEC);
			Serial.print(F(" ("));
			switch (now.dayOfTheWeek())
			{
			case 0:
				Serial.print(F("Sunday"));
				break;
			case 1:
				Serial.print(F("Monday"));
				break;
			case 2:
				Serial.print(F("Tuesday"));
				break;
			case 3:
				Serial.print(F("Wednesday"));
				break;
			case 4:
				Serial.print(F("Thursday"));
				break;
			case 5:
				Serial.print(F("Friday"));
				break;
			case 6:
				Serial.print(F("Saturday"));
				break;
			}
			Serial.print(F(")"));
			Serial.print(F(" "));
			Serial.print(now.hour(), DEC);
			Serial.print(F(":"));
			Serial.print(now.minute(), DEC);
			Serial.print(F(":"));
			Serial.println(now.second(), DEC);
			return;
		}
		newValue = splitString(data, '=', 1).toInt();
		int configIndex = getIndex(name);
		Configuration c = conf[configIndex];
		if (!getIndex(name))
		{
			Serial.println(F("This parameter doesn't exist !"));
			return;
		}
		else if (c.min > newValue || c.max < newValue)
		{
			Serial.print(F("Place set a number between "));
			Serial.print(c.min);
			Serial.print(F(" and "));
			Serial.println(c.max);
			return;
		}
		setValue(name, newValue);
		Serial.print(F("New value for "));
		Serial.print(name);
		Serial.print(F(" = "));
		Serial.println(newValue);
	}
}