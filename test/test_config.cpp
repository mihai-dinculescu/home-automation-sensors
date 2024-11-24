// #define ARDUINO
// #define UNIT_TEST

#if defined(ARDUINO) && defined(UNIT_TEST)

#include <Arduino.h>
#include <unity.h>

#include "MAD_ESP32.h"
#include "config.h"

void test_config()
{
    TEST_ASSERT_EQUAL("192.168.1.211", config.mqtt_broker);
    TEST_ASSERT_EQUAL(21, *config.sd_chip_select);
}

void test_board()
{
    TEST_ASSERT_EQUAL(26, board.pins.A0);
}

void setup()
{
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_config);
    RUN_TEST(test_board);

    UNITY_END();
}

void loop()
{
    // nothing to be done here.
}

#endif
