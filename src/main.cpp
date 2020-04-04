#include <sstream>
#include <iostream>
#include <string>

#include "config.h"
#include "MAD_ESP32.h"

#include "messaging.h"
#include "storage.h"
#include "sensor_bsec.h"

#ifdef CAPABILITIES_CONFIG_REMOTE
    #include "config_remote.h"
#endif

#ifdef CAPABILITIES_MOISTURE_SENSOR
    #include "sensor_soil_catnip.h"
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

void HandleFatalError(const char *message)
{
    LOGLNT("%s", message);
    storage.LogError(message);
    board.FatalError();
}

void LogErrorAndSleep(const char *message)
{
    LOGLN(" RESTARTING.");
    storage.LogError(message);

    Serial.flush();
    board.DeepSleep(10);
}

const char* GenerateMessage(uint16_t plant_moisture)
{
    std::ostringstream messageStream;

    messageStream << "{";
    messageStream << "\"location\":\"" << config.mqtt_location << "\"";
    messageStream << ",\"temperature\":" << sensor_bsec.getTemperature();
    messageStream << ",\"humidity\":" << sensor_bsec.getHumidity();
    messageStream << ",\"pressure\":" << sensor_bsec.getPressure() / 100; // hPa
    messageStream << ",\"iaq\":" << sensor_bsec.getIaq();
    messageStream << ",\"iaq_accuracy\":" << sensor_bsec.getIaqAccuracy();
    messageStream << ",\"static_iaq\":" << sensor_bsec.getStaticIaq();
    messageStream << ",\"bvoc_equivalent\":" << sensor_bsec.getBreathVocEquivalent();
    messageStream << ",\"co2_equivalent\":" << sensor_bsec.getCo2Equivalent();

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

    storage.Setup();

    if (!board.SetupWifi(config.wifi_ssid, config.wifi_password)) {
        LogErrorAndSleep("WiFi connect timeout.");
    }

    if (!board.SetupTime()) {
        LogErrorAndSleep("Time fetch timeout.");
    }

    sensor_bsec.Setup();
    messaging.Setup(config.mqtt_broker);

    #ifdef CAPABILITIES_MOISTURE_SENSOR
        if (sensor_soil_catnip.Setup()) {
            LOGLNT("Catnip Soil init done.");
        } else {
            HandleFatalError("Catnip Soil init failed!");
        }
    #endif
}

void loop()
{
    messaging.Loop();
    delay(10);

    if (sensor_bsec.Run(board.GetTimestamp())) {
        LOGLNT("Temperature raw %.2f compensated %.2f", sensor_bsec.getRawTemperature(), sensor_bsec.getTemperature());
        LOGLNT("Humidity raw %.2f %% compensated %.2f %%", sensor_bsec.getRawHumidity(), sensor_bsec.getHumidity());
        LOGLNT("Pressure %.2f kPa", sensor_bsec.getPressure() / 1000);
        LOGLNT("IAQ %.0f accuracy %d", sensor_bsec.getIaq(), sensor_bsec.getIaqAccuracy());
        LOGLNT("Static IAQ %.0f accuracy %d", sensor_bsec.getStaticIaq(), sensor_bsec.getStaticIaqAccuracy());
        LOGLNT("Breath VOC ppm %.0f accuracy %d", sensor_bsec.getBreathVocEquivalent(), sensor_bsec.getBreathVocAccuracy());
        LOGLNT("CO2 %.0f ppm accuracy %d", sensor_bsec.getCo2Equivalent(), sensor_bsec.getCo2Accuracy());
        LOGLNT("Gas resistance %.2f kOhm", sensor_bsec.getGasResistance() / 1000);

        bool hold_pins = false;

        if (SetWarningLed(*config.iaq_warning_pin, sensor_bsec.getIaq() >= config.iaq_warning_threshold)) {
            hold_pins = true;
        }

        #ifdef CAPABILITIES_CONFIG_REMOTE
        #endif

        uint16_t plant_moisture = 0;

        #ifdef CAPABILITIES_MOISTURE_SENSOR
            plant_moisture = sensor_soil_catnip.Read();
            LOGLNT("Plant moisture (Catnip) %d", plant_moisture);
        #endif

        #if defined(CAPABILITIES_CONFIG_REMOTE) && defined(CAPABILITIES_MOISTURE_SENSOR)
            config_remote.Read(config.config_url);

            LOGLNT("Plant moisture threshold %d", config_remote.GetField1());

            if (SetWarningLed(*config.moisture_warning_pin, plant_moisture <= config_remote.GetField1())) {
                hold_pins = true;
            }
        #endif

        if (hold_pins) {
            gpio_deep_sleep_hold_en();
        } else {
            gpio_deep_sleep_hold_dis();
        }

        sensor_bsec.SaveState();

        if (messaging.Connect(config.mqtt_client_id)) {
            const char* message = GenerateMessage(plant_moisture);

            if (!messaging.Publish(config.mqtt_topic, message)) {
                storage.LogError("MQTT publish failed.");
            }
        } else {
            storage.LogError("MQTT connect timeout.");
        }

        storage.End();

        uint64_t time_us = ((sensor_bsec.getNextCall() - board.GetTimestamp()) * 1000) - esp_timer_get_time();
        LOGLNT("Deep sleep for %llu ms. BSEC next call at %llu ms.", time_us / 1000, sensor_bsec.getNextCall());
        board.DeepSleepRaw(time_us);
    }
}
#endif
