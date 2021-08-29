#include <Wire.h>

#include "sensor_soil_catnip.h"

SensorSoilCatnip sensor_soil_catnip;

bool SensorSoilCatnip::Setup()
{
    Wire.begin();
    _sensor.begin();
    delay(1000); // give some time to boot up

    if (Read() == 65535)
    {
        return false;
    }

    return true;
}

uint16_t SensorSoilCatnip::Read()
{
    while (_sensor.isBusy())
    {
        delay(50);
    }

    uint16_t value = _sensor.getCapacitance();
    _sensor.sleep();

    return value;
}
