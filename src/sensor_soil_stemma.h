#ifndef SENSOR_SOIL_STEMMA_H
    #define SENSOR_SOIL_STEMMA_H

    #include <Adafruit_seesaw.h>

    class SensorSoilStemma
    {
        Adafruit_seesaw _seesaw_soil;

        public:
            void Setup(const uint8_t addr);
            uint16_t Read();
    };

    extern SensorSoilStemma sensor_soil_stemma;
#endif
