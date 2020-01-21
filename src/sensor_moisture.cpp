#include "config.h"

#ifdef CAPABILITIES_MOISTURE_SENSOR
    #include <ArduinoJson.h>

    #include "sensor_moisture.h"
    #include "MAD_ESP32.h"

    Adafruit_seesaw   seesaw_soil;
    WiFiClientSecure  http_client;

    void SetupMoistureSensor()
    {
        if (!seesaw_soil.begin(config.seesaw_soil_i2c_addr)) {
            LOGLN("Failed to init Seesaw soil!");
            board.FatalError();
        }
        LOGLN("Seesaw Soil init done.");
    }

    uint16_t ReadMoisture()
    {
        return seesaw_soil.touchRead(0);
    }

    void ReadConfig(uint16_t *out_moisture_threshold)
    {
        HTTPClient http_client;

        http_client.begin(config.config_url);

        int response_code = http_client.GET();

        if(response_code > 0) {
            String result = http_client.getString();

            const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(8) + 120 * 3;
            StaticJsonDocument<capacity> doc;

            DeserializationError error = deserializeJson(doc, result);

            if (error) {
                LOGLN("deserializeJson() failed: %s!", error.c_str());
            }

            *out_moisture_threshold = doc["feeds"][0]["field1"];
        } else {
            LOGLN("Error in getting config: %d!", response_code);
        }
    }

    void SetPlantMoistureLed(uint16_t plant_moisture)
    {
        uint16_t plant_moisture_threshold = 0;
        ReadConfig(&plant_moisture_threshold);
        LOGLN("Plant moisture threshold %d", plant_moisture_threshold);

        pinMode(*config.moisture_warning_pin, OUTPUT);
        if (plant_moisture <= plant_moisture_threshold) {
            digitalWrite(*config.moisture_warning_pin, HIGH);
            gpio_hold_en((gpio_num_t)*config.moisture_warning_pin);
            gpio_deep_sleep_hold_en();
        } else {
            digitalWrite(*config.moisture_warning_pin, LOW);
            gpio_hold_dis((gpio_num_t)*config.moisture_warning_pin);
            gpio_deep_sleep_hold_dis();
        }
    }
#endif