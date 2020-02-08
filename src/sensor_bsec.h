#ifndef SENSOR_BSEC_H
    #define SENSOR_BSEC_H

    #include <bsec.h>

    class SensorBsec
    {
        Bsec _sensor;

        bool CheckSensor();
        void DumpState(const char *name, const uint8_t *state);

        public:
            void Setup();
            void SaveState();

            bool Run(int64_t timeMilliseconds = -1);

            float getRawTemperature();
            float getTemperature();
            float getRawHumidity();
            float getHumidity();
            float getPressure();
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
