#include <Arduino.h>
#include "sensor_soil_gravity.h"

SensorSoilGravity sensor_soil_gravity;

bool SensorSoilGravity::Setup(const uint8_t addr)
{
    _analog_addr = addr;

    // first read is funny, ignore it
    Read();
    delay(500);

    return Read() < 4095;
}

uint16_t SensorSoilGravity::Read()
{
    // the sensor gives the dryness measure
    // substract it from the max value so that we get the moisture measure
    return 4095 - analogRead(_analog_addr);
}
