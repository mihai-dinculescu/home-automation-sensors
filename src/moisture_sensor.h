#ifndef MOISTURE_SENSOR_H
    #define MOISTURE_SENSOR_H

    #include "Adafruit_seesaw.h"
    #include <HTTPClient.h>
    #include <ArduinoJson.h>

    extern Adafruit_seesaw   seesaw_soil;
    extern WiFiClientSecure  http_client;

    void SetupMoistureSensor();
    uint16_t ReadMoisture();
    void SetPlantMoistureLed(uint16_t plant_moisture);
#endif
