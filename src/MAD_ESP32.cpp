#include "MAD_ESP32.h"

const Pins pins = {
    13,  // LED_ERROR
    26,  // A0
    25,  // A1
    21,  // P21
    23,  // I2C_SDA
    22,  // I2C_SCL
};

WiFiClient wifi_client;

int64_t GetTimestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

void SetupWifi(const char* ssid, const char* password)
{
    LOG("Connecting to %s.", ssid);
    WiFi.begin(ssid, password);

    uint64_t time_ms = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() >= time_ms + 30 * 1000) {
            #ifdef CAPABILITIES_SD
                LogRestart("WiFi connect timeout.");
            #endif

            LOGLN("RESTARTING.");
            Serial.flush();
            DeepSleep(10);
        }
        LOG(".");
        delay(500);
    }

    LOG(" CONNECTED.\n");
}

void SetupTime()
{
    configTime(0, 0, "pool.ntp.org", "time.windows.com", "time.nist.gov");
    struct tm time_info;

    LOG("Getting time.");
    uint16_t retries = 0;
    while (!getLocalTime(&time_info)) {
        if (retries >= 1) {
            #ifdef CAPABILITIES_SD
                LogRestart("Time fetch timeout.");
            #endif

            LOGLN("RESTARTING.");
            Serial.flush();
            DeepSleep(10);
        }
        configTime(0, 0, "pool.ntp.org", "time.windows.com", "time.nist.gov");
        retries++;
        LOG(".");
    }

    char datetime_buffer[128];
    strftime(datetime_buffer, sizeof(datetime_buffer), "%A, %B %d %Y %H:%M:%S", &time_info);

    LOG(" Current time is: %s.\n", datetime_buffer);
}

void BlinkErrorLed(uint16_t interval)
{
    pinMode(pins.LED_ERROR, OUTPUT);
    digitalWrite(pins.LED_ERROR, HIGH);
    delay(interval);
    digitalWrite(pins.LED_ERROR, LOW);
    delay(interval);
}

void FatalError()
{
    while(true) {
        BlinkErrorLed(100);
    }
}

void DeepSleep(uint16_t seconds)
{
    uint64_t time_us = seconds * 1000 * 1000ull - esp_timer_get_time();
    LOGLN("Deep sleep for %llu ms!", time_us / 1000);
    esp_sleep_enable_timer_wakeup(time_us);
    esp_deep_sleep_start();
}
