#ifndef SENSOR_MOISTURE_H
    #define SENSOR_MOISTURE_H

    #include <Adafruit_seesaw.h>
    #include <HTTPClient.h>

    extern Adafruit_seesaw   seesaw_soil;
    extern WiFiClientSecure  http_client;

    void SetupMoistureSensor();
    uint16_t ReadMoisture();
    void ReadMoistureConfig(uint16_t *out_moisture_threshold);
#endif
