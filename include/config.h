#ifndef CONFIG_H
    #define CONFIG_H

    #include <stdint.h>

    // 0 - Living Room
    // 1 - Master Bedroom
    #define SENSOR_LOCATION 1

    // #define BSEC_DUNP_STATE

    extern const char*          wifi_ssid;
    extern const char*          wifi_password;
    extern const char*          mqtt_broker;

    extern const char*          mqtt_location;
    extern const char*          mqtt_client_id;
    extern const char*          mqtt_topic;

    #if SENSOR_LOCATION == 0
        #define CAPABILITIES_DISPLAY
    #elif SENSOR_LOCATION == 1
        #define CAPABILITIES_MOISTURE_SENSOR
    #endif

    #ifdef CAPABILITIES_DISPLAY
        extern const uint8_t    ssd1306_i2c_addr;
    #endif

    #ifdef CAPABILITIES_MOISTURE_SENSOR
        extern const uint8_t    seesaw_soil_addr;
        extern const uint8_t    moisture_warning_pin;

        extern const char*      config_url;
    #endif
#endif
