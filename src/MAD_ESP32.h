#ifndef MAD_ESP32_H
    #define MAD_ESP32_H

    #include <stdint.h>
    #include <sys/time.h>

    #include <WiFi.h>

    #define LOG(fmt, ...) (Serial.printf(fmt, ##__VA_ARGS__))
    #define LOGLN(fmt, ...) (Serial.printf("%09llu: " fmt "\n", GetTimestamp(), ##__VA_ARGS__))

    struct Pins {
        const uint16_t A0;
        const uint16_t I2C_SDA;
        const uint16_t I2C_SCL;
        const uint16_t LED_ERROR;
    };

    extern const Pins pins;
    extern WiFiClient wifi_client;

    int64_t GetTimestamp();
    void SetupWifi(const char* ssid, const char* password);
    void SetupTime();
    void BlinkErrorLed(uint16_t interval = 500);
    void FatalError();
    void DeepSleep(uint16_t seconds);
#endif
