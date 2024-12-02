#ifndef SENSOR_BSEC_H
#define SENSOR_BSEC_H

#include <bsec2.h>

class SensorBsec
{
    Bsec2 _sensor;

    float _raw_temperature;
    float _temperature;
    float _raw_humidity;
    float _humidity;
    float _raw_pressure;
    float _iaq;
    uint16_t _iaq_accuracy;
    float _static_iaq;
    uint16_t _static_iaq_accuracy;
    float _breath_voc_equivalent;
    uint16_t _breath_voc_accuracy;
    float _co2_equivalent;
    uint16_t _co2_accuracy;

    bool CheckSensor();
    void DumpState(const char *name, const uint8_t *state);

public:
    bool Setup();
    bool SaveState();

    bool Run(int64_t timeMilliseconds = -1);

    float getRawTemperature();
    float getTemperature();
    float getRawHumidity();
    float getHumidity();
    float getRawPressure();
    float getIaq();
    uint16_t getIaqAccuracy();
    float getStaticIaq();
    uint16_t getStaticIaqAccuracy();
    float getBreathVocEquivalent();
    uint16_t getBreathVocAccuracy();
    float getCo2Equivalent();
    uint16_t getCo2Accuracy();
    float getGasResistance();

    int64_t getNextCall();
};

extern SensorBsec sensor_bsec;
#endif
