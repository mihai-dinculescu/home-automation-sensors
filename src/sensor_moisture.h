#ifndef SENSOR_MOISTURE_H
    #define SENSOR_MOISTURE_H

    #include <Adafruit_seesaw.h>

    class SensorMoisture
    {
        Adafruit_seesaw _seesaw_soil;

        public:
            void Setup(const uint8_t addr);
            uint16_t Read();
    };

    extern SensorMoisture sensor_moisture;
#endif
