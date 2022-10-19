#ifndef __Config_h__
#define __Config_h__

#include "Arduino.h"
#include "EEPROM.h"

#include "../RTClib/RTClib.h"

typedef struct
{
	int value;
	String name;
	int min;
	int max;
	bool isByte;
	byte size;
	int eepromAdr;

} Configuration;

class Config
{

public:
	Config(byte version, String batchNumber);

	void waitValues();
	long getlastActivity();
	int getValue(String name);
	void setValue(String name, int newValue);
	void showValues();
	void getVersion();
	void resetValues();

	// long getLastActivity();

private:
	String _batchNumber;
	byte getIndex(String name);
	byte getSize(int index);

	int read(int index, int _size);
	void writeValues(int index, int value);

	// RTC_DS1307 time;
};

#endif
