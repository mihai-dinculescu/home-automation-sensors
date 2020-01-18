#ifndef LOGGING_H
    #define LOGGING_H

    #include <SD.h>

    #include "config.h"
    #include "MAD_ESP32.h"

    void SetupSD();
    void LogRestart(const char* message);
    void DataWrite(const uint16_t len, const uint8_t* state);
    bool DataRead(const uint16_t len, uint8_t* state);
    void DataDelete();
#endif
