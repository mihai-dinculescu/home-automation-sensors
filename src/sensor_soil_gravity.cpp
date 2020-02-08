#include <Arduino.h>
#include "sensor_soil_gravity.h"

SensorSoilGravity sensor_soil_gravity;

bool SensorSoilGravity::Setup(const uint8_t addr)
{
    _analog_addr = addr;

    // should be 4095 but there can be noise in the wire
    return Read() < 3500;
}

uint16_t SensorSoilGravity::Read()
{
    uint16_t samples = 128;
    uint64_t accumulator = 0;

    for (uint16_t i = 0; i < samples; i++) {
        delay(1);
        accumulator += analogRead(_analog_addr);
    }

    // the sensor gives the dryness measure
    // substract it from the max value so that we get the moisture measure
    return 4095 - accumulator / samples;
}
