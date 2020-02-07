#ifndef SENSOR_SOIL_GRAVITY_H
    #define SENSOR_SOIL_GRAVITY_H

    #include <stdint.h>

    class SensorSoilGravity
    {
        uint8_t _analog_addr;

        public:
            bool Setup(const uint8_t addr);
            uint16_t Read();
    };

    extern SensorSoilGravity sensor_soil_gravity;
#endif
