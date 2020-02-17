#include <SD.h>

#include "config.h"
#include "MAD_ESP32.h"

#include "storage.h"

Storage storage;

void HandleStorageError()
{
    // blink for 10 seconds
    for (uint16_t i = 0; i < 50; i++) {
        board.BlinkErrorLed(100);
    }

    board.Restart();
}

void Storage::Setup()
{
    LOGLNT("Initializing SD Card...");

    pinMode(*config.sd_chip_select, OUTPUT);

    digitalWrite(*config.sd_chip_select, HIGH);

    uint64_t time_ms = millis();
    while (!SD.begin(*config.sd_chip_select)) {
        if (millis() >= time_ms + 10 * 1000) {
            LOGLNT("SD Card initialization FAILED!");
            HandleStorageError();
        }
        delay(500);
    }

    if (!SD.exists("/logs")) {
        SD.mkdir("/logs");
    }

    if (!SD.exists("/data")) {
        SD.mkdir("/data");
    }

    LOGLNT("SD Card initialization DONE.");
}

void Storage::End()
{
    SD.end();
}

void Storage::LogError(const char *message)
{
    File sd_file;
    sd_file = SD.open("/logs/errors.txt", FILE_APPEND);

    if (sd_file) {
        LOGT("Writing to /logs/errors.txt.");
        sd_file.printf("%09llu: %s\n", board.GetTimestamp(), message);
        sd_file.flush();
        sd_file.close();
        LOGLN(" DONE.");
    } else {
        LOGLNT("Failed to write to SD Card!");
        HandleStorageError();
    }
}

void Storage::ConfigWrite(const uint16_t len, const uint8_t *state)
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
        LOGLNT("Failed to write to SD Card!");
        HandleStorageError();
    }
}

bool Storage::ConfigRead(const uint16_t len, uint8_t *state)
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

void Storage::ConfigDelete()
{
    if (!SD.exists("/data/data.txt")) {
        return;
    }

    SD.remove("/data/data.txt");
}
