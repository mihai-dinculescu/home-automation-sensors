#include "sensor_moisture.h"
#include "MAD_ESP32.h"

SensorMoisture sensor_moisture;

void SensorMoisture::Setup(const uint8_t addr)
{
    if (!_seesaw_soil.begin(addr)) {
        LOGLNT("Failed to init Seesaw soil!");
        board.FatalError();
    }
    LOGLNT("Seesaw Soil init done.");
}

uint16_t SensorMoisture::Read()
{
    return _seesaw_soil.touchRead(0);
}
