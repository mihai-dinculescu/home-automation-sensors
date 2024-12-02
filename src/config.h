#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#include "config_secrets.h"
#include "MAD_ESP32.h"

// 1 - Living Room
// 2 - Master Bedroom
// 3 - Nursery
// 4 - Office #1
// 5 - OFfice #2
#define SENSOR_LOCATION 1

struct Config
{
    const char *wifi_ssid = config_secrets.wifi_ssid;
    const char *wifi_password = config_secrets.wifi_password;
    const char *mqtt_broker = config_secrets.mqtt_broker;

#if SENSOR_LOCATION == 1
    const char *mqtt_location = "Living Room";
    const char *mqtt_client_id = "mqtt_client_living_room";
    const char *mqtt_topic = "sensors/living_room";
#elif SENSOR_LOCATION == 2
    const char *mqtt_location = "Master Bedroom";
    const char *mqtt_client_id = "mqtt_client_bedroom_master";
    const char *mqtt_topic = "sensors/bedroom_master";
#elif SENSOR_LOCATION == 3
    const char *mqtt_location = "Nursery";
    const char *mqtt_client_id = "mqtt_client_nursery";
    const char *mqtt_topic = "sensors/nursery";
#elif SENSOR_LOCATION == 4
    const char *mqtt_location = "Office #1";
    const char *mqtt_client_id = "mqtt_client_office_1";
    const char *mqtt_topic = "sensors/office_1";
#elif SENSOR_LOCATION == 5
    const char *mqtt_location = "OFfice #2";
    const char *mqtt_client_id = "mqtt_client_office_2";
    const char *mqtt_topic = "sensors/office_2";
#endif

    const uint16_t sd_chip_select = board.pins.P21;

    const uint16_t iaq_warning_threshold = 151;
    const uint16_t iaq_warning_pin = board.pins.A1;
};

extern Config config;
#endif
