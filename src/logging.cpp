#include <SD.h>

#include "config.h"
#include "MAD_ESP32.h"

#ifdef CAPABILITIES_SD
    #include "logging.h"

    void SetupSD()
    {
        LOG("Initializing SD card. ");

        pinMode(*config.sd_chip_select, OUTPUT);

        digitalWrite(*config.sd_chip_select, HIGH);

        uint64_t time_ms = millis();
        if (!SD.begin(*config.sd_chip_select)) {
            if (millis() >= time_ms + 10 * 1000) {
                board.FatalError();
            }
            LOG(".");
            delay(500);
        }

        if (!SD.exists("/logs")) {
            SD.mkdir("/logs");
        }

        if (!SD.exists("/data")) {
            SD.mkdir("/data");
        }

        LOG("DONE.\n");
    }

    void LogRestart(const char *message)
    {
        File sd_file;
        sd_file = SD.open("/logs/restart.txt", FILE_APPEND);

        if (sd_file) {
            Serial.print("Writing to /logs/restart.txt.");
            sd_file.printf("%09llu: %s\n", board.GetTimestamp(), message);
            sd_file.flush();
            sd_file.close();
            Serial.println(" DONE.");
        } else {
            LOGLN("Failed to write to SD card!");
            board.FatalError();
        }
    }

    void DataWrite(const uint16_t len, const uint8_t *state)
    {
        File sd_file;
        sd_file = SD.open("/data/data.txt", FILE_WRITE);

        if (sd_file) {
            for (int i = 0; i < len; i++) {
                sd_file.println(state[i]);
            }

            sd_file.flush();
            sd_file.close();
        } else {
            LOGLN("Failed to write to SD card!");
            board.FatalError();
        }
    }

    bool DataRead(const uint16_t len, uint8_t *state)
    {
        File sd_file;

        if (!SD.exists("/data/data.txt")) {
            return false;
        }

        sd_file = SD.open("/data/data.txt", FILE_READ);

        if (sd_file) {
            for (int i = 0; i < len; i++) {
                if (sd_file.available()) {
                    const int x = sd_file.readStringUntil('\n').toInt();
                    state[i] = x;
                } else {
                    sd_file.close();
                    return false;
                }
            }

            sd_file.close();
        } else {
            return false;
        }

        return true;
    }

    void DataDelete()
    {
        if (!SD.exists("/data/data.txt")) {
            return;
        }

        SD.remove("/data/data.txt");
    }
#endif