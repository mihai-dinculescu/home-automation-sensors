#ifndef SENSOR_SOIL_CATNIP_H
    #define SENSOR_SOIL_CATNIP_H

    #include <I2CSoilMoistureSensor.h>

    class SensorSoilCatnip
    {
        I2CSoilMoistureSensor _sensor;

        public:
            bool Setup();
            uint16_t Read();
    };

    extern SensorSoilCatnip sensor_soil_catnip;
#endif
