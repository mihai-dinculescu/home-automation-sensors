#include "config_secrets.h"
#include "config.h"
#include "MAD_ESP32.h"

const char*          wifi_ssid               = config_secrets.wifi_ssid;
const char*          wifi_password           = config_secrets.wifi_password;
const char*          mqtt_broker             = "192.168.1.100";

#if SENSOR_LOCATION == 0
    const char*      mqtt_location           = "Living Room";
    const char*      mqtt_client_id          = "mqtt_client_living_room";
    const char*      mqtt_topic              = "sensors/living_room";
#elif SENSOR_LOCATION == 1
    const char*      mqtt_location           = "Master Bedroom";
    const char*      mqtt_client_id          = "mqtt_client_bedroom_master";
    const char*      mqtt_topic              = "sensors/bedroom_master";
#endif

#ifdef CAPABILITIES_DISPLAY
    const uint8_t    ssd1306_i2c_addr        = 0x3C;
#endif

#ifdef CAPABILITIES_MOISTURE_SENSOR
    const uint8_t    seesaw_soil_addr        = 0x36;
    const uint8_t    moisture_warning_pin    = pins.A0;
    const char*      config_url              = config_secrets.config_url;
#endif
