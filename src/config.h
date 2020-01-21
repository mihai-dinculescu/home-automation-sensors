#ifndef CONFIG_H
    #define CONFIG_H

    #include <stdint.h>

    #include "config_secrets.h"
    #include "MAD_ESP32.h"

    // 0 - Living Room
    // 1 - Master Bedroom
    #define SENSOR_LOCATION 1

    // #define BSEC_DUNP_STATE

    #if SENSOR_LOCATION == 0
        #define CAPABILITIES_DISPLAY
    #elif SENSOR_LOCATION == 1
        #define CAPABILITIES_SD
        #define CAPABILITIES_MOISTURE_SENSOR
    #endif

    // #define CAPABILITIES_MQTT_CONFIG

    struct Config {
        const char           *wifi_ssid               = config_secrets.wifi_ssid;
        const char           *wifi_password           = config_secrets.wifi_password;
        const char           *mqtt_broker             = "192.168.1.100";

        #if SENSOR_LOCATION == 0
            const char       *mqtt_location           = "Living Room";
            const char       *mqtt_client_id          = "mqtt_client_living_room";
            const char       *mqtt_topic              = "sensors/living_room";
            #ifdef CAPABILITIES_MQTT_CONFIG
                const char   *mqtt_topic_memory       = "memory/sensors/living_room";
            #endif
        #elif SENSOR_LOCATION == 1
            const char       *mqtt_location           = "Master Bedroom";
            const char       *mqtt_client_id          = "mqtt_client_bedroom_master";
            const char       *mqtt_topic              = "sensors/bedroom_master";
            #ifdef CAPABILITIES_MQTT_CONFIG
                const char   *mqtt_topic_memory       = "memory/sensors/bedroom_master";
            #endif
        #endif

        #ifdef CAPABILITIES_DISPLAY
            const uint8_t    ssd1306_i2c_addr        = 0x3C;
        #endif

        #ifdef CAPABILITIES_SD
            const uint16_t   *sd_chip_select          = &board.pins.P21;
        #endif

        #ifdef CAPABILITIES_MOISTURE_SENSOR
            const uint8_t    seesaw_soil_i2c_addr     = 0x36;
            const uint16_t   *moisture_warning_pin    = &board.pins.A0;
            const char       *config_url              = config_secrets.config_url;
        #endif
    };

    extern Config config;
#endif