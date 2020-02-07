#include "sensor_soil_stemma.h"
#include "MAD_ESP32.h"

SensorSoilStemma sensor_soil_stemma;

void SensorSoilStemma::Setup(const uint8_t addr)
{
    if (!_seesaw_soil.begin(addr)) {
        LOGLNT("Failed to init Seesaw soil!");
        board.FatalError();
    }
    LOGLNT("Seesaw Soil init done.");
}

uint16_t SensorSoilStemma::Read()
{
    return _seesaw_soil.touchRead(0);
}
