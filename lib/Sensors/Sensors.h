#ifndef __Sensors_h__
#define __Sensors_h__

#include "Arduino.h"
#include "Config.h"

#include "../RTClib/RTClib.h"

#include <SPI.h>
#include <SD.h>
#include <EnvironmentCalculations.h>
#include <Wire.h>
#include <BME280.h>
#include <BME280I2C.h>
#include <BME280I2C_BRZO.h>
#include <BME280Spi.h>
#include <BME280SpiSw.h>

#define LIGHT_PIN 0
// #define MAX_VALUES 3

typedef struct
{
    char id;
    String name;
    bool error;
    float value;

} Sensors;

class Sensor
{

public:
    Sensor();

    void initialize();
    void showValues();
    byte setValues();
    void writetoSD(bool write);

private:
    String changefileName();
};

#endif