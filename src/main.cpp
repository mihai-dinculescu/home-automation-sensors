#include <sstream>
#include <iostream>
#include <string>

#include "config.h"
#include "MAD_ESP32.h"

#include "messaging.h"
#include "storage.h"
#include "sensor_bsec.h"

bool SetWarningLed(uint16_t pin_address, bool show_warning, bool quiet_hours)
{
    pinMode(pin_address, OUTPUT);

    if (show_warning && !quiet_hours)
    {
        digitalWrite(pin_address, HIGH);
        gpio_hold_en((gpio_num_t)pin_address);
        return true;
    }
    else
    {
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

const char *GenerateMessage()
{
    std::ostringstream messageStream;

    messageStream << "{";
    messageStream << "\"location\":\"" << config.mqtt_location << "\"";
    messageStream << ",\"temperature\":" << sensor_bsec.getTemperature();
    messageStream << ",\"humidity\":" << sensor_bsec.getHumidity();
    messageStream << ",\"pressure\":" << sensor_bsec.getRawPressure();
    messageStream << ",\"iaq\":" << sensor_bsec.getIaq();
    messageStream << ",\"iaq_accuracy\":" << sensor_bsec.getIaqAccuracy();
    messageStream << ",\"static_iaq\":" << sensor_bsec.getStaticIaq();
    messageStream << ",\"bvoc_equivalent\":" << sensor_bsec.getBreathVocEquivalent();
    messageStream << ",\"co2_equivalent\":" << sensor_bsec.getCo2Equivalent();

    messageStream << "}";

    std::string messageString = messageStream.str();
    const char *messageChar = messageString.c_str();

    return messageChar;
}

#ifndef UNIT_TEST
void setup()
{
    Serial.begin(115200);

    storage.Setup();

    if (!board.SetupWifi(config.wifi_ssid, config.wifi_password))
    {
        LogErrorAndSleep("WiFi connect timeout.");
    }

    if (!board.SetupTime())
    {
        LogErrorAndSleep("Time fetch timeout.");
    }

    if (!sensor_bsec.Setup())
    {
        HandleFatalError("BSEC init failed.");
    }

    messaging.Setup(config.mqtt_broker);
}

void loop()
{
    messaging.Loop();
    delay(10);

    if (sensor_bsec.Run(board.GetTimestamp()))
    {
        time_t currentTime = time(NULL);
        struct tm *localTime = localtime(&currentTime);

        uint16_t hour = localTime->tm_hour;
        bool quiet_hours = hour >= 23 || hour <= 7;

        bool hold_pins = false;

        if (SetWarningLed(*config.iaq_warning_pin, sensor_bsec.getIaq() >= config.iaq_warning_threshold, quiet_hours))
        {
            hold_pins = true;
        }

        if (hold_pins)
        {
            gpio_deep_sleep_hold_en();
        }
        else
        {
            gpio_deep_sleep_hold_dis();
        }

        sensor_bsec.SaveState();

        if (messaging.Connect(config.mqtt_client_id))
        {
            const char *message = GenerateMessage();

            if (!messaging.Publish(config.mqtt_topic, message))
            {
                storage.LogError("MQTT publish failed.");
            }
        }
        else
        {
            storage.LogError("MQTT connect timeout.");
        }

        storage.End();

        uint64_t time_us = ((sensor_bsec.getNextCall() - board.GetTimestamp()) * 1000LL) - esp_timer_get_time() - 15 * 1000LL * 1000LL;
        LOGLNT("Deep sleep for %llu ms. BSEC next call at %llu ms.", time_us / 1000LL, sensor_bsec.getNextCall());
        board.DeepSleepRaw(time_us);
    }
}
#endif
