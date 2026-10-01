#ifndef BSEC2_EXT_H
#define BSEC2_EXT_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <bme68xLibrary.h>

// Bsec2 keeps everything needed to drive it with an external clock private.
// Open it up for the subclass below instead of patching the library in .pio.
// The headers above are included first so that only bsec2.h is affected.
#define private protected
#include <bsec2.h>
#undef private

// Bsec2 with a caller-provided clock, so that the BSEC timing survives deep sleep.
class Bsec2Ext : public Bsec2
{
public:
    /**
     * @brief Same as Bsec2::run, but uses the given timestamp instead of millis()
     * @param timeMilliseconds : Current time in milliseconds
     * @return	true for success, false otherwise
     */
    bool run(int64_t timeMilliseconds);

    /**
     * @brief Function to get the time of the next required call to run, in microseconds
     */
    int64_t getNextCall(void);
};

#endif
