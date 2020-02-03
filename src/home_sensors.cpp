#include <sstream>
#include <iostream>
#include <string>

#include "config.h"
#include "MAD_ESP32.h"
#include "messaging.h"

#include "sensor_bsec.h"

#ifdef CAPABILITIES_SD
    #include "logging.h"
#endif

#ifdef CAPABILITIES_MOISTURE_SENSOR
    #include "sensor_moisture.h"
#endif

bool SetWarningLed(uint16_t pin_address, bool show_warning)
{
    pinMode(pin_address, OUTPUT);
    if (show_warning) {
        digitalWrite(pin_address, HIGH);
        gpio_hold_en((gpio_num_t)pin_address);
        return true;
    } else {
        digitalWrite(pin_address, LOW);
        gpio_hold_dis((gpio_num_t)pin_address);
        return false;
    }
}

void LogErrorAndSleep(const char *message)
{
    LOGLN(" RESTARTING.");

    #ifdef CAPABILITIES_SD
        LogError(message);
    #endif

    Serial.flush();
    board.DeepSleep(10);
}

const char* GenerateMessage(float temperature, float humidity, float pressure, float iaq_estimate, uint16_t iaq_accuracy, uint16_t plant_moisture)
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

    std::string messageString = messageStream.str();
    const char* messageChar = messageString.c_str();

    return messageChar;
}

#ifndef UNIT_TEST
void setup()
{
    Serial.begin(115200);

    #ifdef CAPABILITIES_SD
        SetupSD();
    #endif

    if (!board.SetupWifi(config.wifi_ssid, config.wifi_password)) {
        LogErrorAndSleep("WiFi connect timeout.");
    }

    if (!board.SetupTime()) {
        LogErrorAndSleep("Time fetch timeout.");
    }

    SetupBsec();
    messaging.Setup(config.mqtt_broker);

    #ifdef CAPABILITIES_MOISTURE_SENSOR
        SetupMoistureSensor();
    #endif
}

void loop()
{
    messaging.Loop();
    delay(10);

    if (sensor.run(board.GetTimestamp())) {
        LOGLNT("Temperature raw %.2f compensated %.2f", sensor.rawTemperature, sensor.temperature);
        LOGLNT("Humidity raw %.2f compensated %.2f", sensor.rawHumidity, sensor.humidity);
        LOGLNT("Pressure %.2f kPa", sensor.pressure / 1000);
        LOGLNT("IAQ %.0f accuracy %d", sensor.iaq, sensor.iaqAccuracy);
        LOGLNT("Static IAQ %.0f accuracy %d", sensor.staticIaq, sensor.staticIaqAccuracy);
        LOGLNT("Gas resistance %.2f kOhm", sensor.gasResistance / 1000);

        bool hold_pins = false;

        #ifdef CAPABILITIES_IAQ_WARNING
            if (SetWarningLed(*config.iaq_warning_pin, sensor.iaq >= config.iaq_warning_threshold)) {
                hold_pins = true;
            }
        #endif

        uint16_t plant_moisture = 0;
        #ifdef CAPABILITIES_MOISTURE_SENSOR
            plant_moisture = ReadMoisture();
            LOGLNT("Plant moisture %d", plant_moisture);
            uint16_t plant_moisture_threshold = 0;

            ReadMoistureConfig(&plant_moisture_threshold);
            LOGLNT("Plant moisture threshold %d", plant_moisture_threshold);

            if (SetWarningLed(*config.moisture_warning_pin, plant_moisture <= plant_moisture_threshold)) {
                hold_pins = true;
            }
        #endif

        if (hold_pins) {
            gpio_deep_sleep_hold_en();
        } else {
            gpio_deep_sleep_hold_dis();
        }

        SaveBsecState();

        if (messaging.Connect(config.mqtt_client_id)) {
            const char* message = GenerateMessage(sensor.temperature, sensor.humidity, sensor.pressure, sensor.iaq, sensor.iaqAccuracy, plant_moisture);

            if (!messaging.Publish(config.mqtt_topic, message)) {
                LogError("MQTT publish failed.");
            }
        } else {
            LogError("MQTT connect timeout.");
        }

        uint64_t time_us = ((sensor.nextCall - board.GetTimestamp()) * 1000) - esp_timer_get_time();
        LOGLNT("Deep sleep for %llu ms. BSEC next call at %llu ms.", time_us / 1000, sensor.nextCall);
        board.DeepSleepRaw(time_us);
    }
}
#endif
