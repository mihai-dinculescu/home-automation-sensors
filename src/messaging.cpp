#include <sstream>
#include <iostream>
#include <string>

#include "config.h"
#include "MAD_ESP32.h"
#include "logging.h"
#include "messaging.h"

#ifdef CAPABILITIES_MQTT_CONFIG
    MQTTClient mqtt_client(834);
#else
    MQTTClient mqtt_client(256);
#endif

void SetupMQTT()
{
    mqtt_client.begin(config.mqtt_broker, board.wifi_client);
    LOGLN("MQTT init done.");
}

void LogData(float temperature, float humidity, float pressure, float iaq_estimate, uint16_t iaq_accuracy, uint16_t plant_moisture)
{
    std::ostringstream messageStream;

    messageStream << "{";
    messageStream << "\"location\":\"" << config.mqtt_location << "\"";
    messageStream << ",\"temperature\":" << temperature;
    messageStream << ",\"humidity\":" << humidity;
    messageStream << ",\"pressure\":" << pressure / 100; // hPa
    messageStream << ",\"iaq_estimate\":" << iaq_estimate;
    messageStream << ",\"iaq_accuracy\":" << iaq_accuracy;

    #ifdef CAPABILITIES_MOISTURE_SENSOR
        messageStream << ",\"plant_moisture\":" << plant_moisture;
    #endif

    messageStream << "}";

    LOG("Connecting to MQTT.");
    uint64_t time_ms = millis();
    while (!mqtt_client.connect(config.mqtt_client_id)) {
        if (millis() >= time_ms + 10 * 1000) {
            #ifdef CAPABILITIES_SD
                LogRestart("MQTT connect timeout.");
            #endif

            LOGLN("RESTARTING.");
            Serial.flush();
            board.DeepSleep(10);
        }
        LOG(".");
        delay(500);
    }
    LOG(" CONNECTED.\n");

    LOG("Publishing to MQTT.");
    mqtt_client.publish(config.mqtt_topic, messageStream.str().c_str(), false, 2);
    LOG(" PUBLISHED with status %d.\n", mqtt_client.lastError());
}
