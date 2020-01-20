#ifndef LOGGING_H
    #define LOGGING_H

    void SetupSD();
    void LogRestart(const char *message);
    void DataWrite(const uint16_t len, const uint8_t *state);
    bool DataRead(const uint16_t len, uint8_t *state);
    void DataDelete();
#endif
