#include "config.h"
#include "MAD_ESP32.h"

#include "sensor_bsec.h"

#ifdef CAPABILITIES_MOISTURE_SENSOR
    #include "sensor_moisture.h"
#endif

#include "messaging.h"

#ifdef CAPABILITIES_SD
    #include "logging.h"
#endif

#ifdef CAPABILITIES_DISPLAY
    #include "display.h"
#endif

#ifndef UNIT_TEST
void setup()
{
    Serial.begin(115200);

    #ifdef CAPABILITIES_SD
        SetupSD();
    #endif

    board.SetupWifi(config.wifi_ssid, config.wifi_password);
    board.SetupTime();
    SetupBsec();
    SetupMQTT();

    #ifdef CAPABILITIES_MOISTURE_SENSOR
        SetupMoistureSensor();
    #endif

    #ifdef CAPABILITIES_DISPLAY
        SetupDisplay();
    #endif
}

void loop()
{
    mqtt_client.loop();
    delay(10);

    if (sensor.run()) {
        LOGLN("Temperature raw %.2f compensated %.2f", sensor.rawTemperature, sensor.temperature);
        LOGLN("Humidity raw %.2f compensated %.2f", sensor.rawHumidity, sensor.humidity);
        LOGLN("Pressure %.2f kPa", sensor.pressure / 1000);
        LOGLN("IAQ %.0f accuracy %d", sensor.iaq, sensor.iaqAccuracy);
        LOGLN("Static IAQ %.0f accuracy %d", sensor.staticIaq, sensor.staticIaqAccuracy);
        LOGLN("Gas resistance %.2f kOhm", sensor.gasResistance / 1000);

        uint16_t plant_moisture = 0;
        #ifdef CAPABILITIES_MOISTURE_SENSOR
            plant_moisture = ReadMoisture();
            LOGLN("Plant moisture %d", plant_moisture);

            SetPlantMoistureLed(plant_moisture);
        #endif

        SaveBsecState();

        LogData(sensor.temperature, sensor.humidity, sensor.pressure, sensor.iaq, sensor.iaqAccuracy, plant_moisture);

        #ifdef CAPABILITIES_DISPLAY
            DisplayData(sensor.temperature, sensor.humidity, sensor.pressure, sensor.iaq, sensor.iaqAccuracy, plant_moisture);
        #endif

        board.DeepSleep(5 * 60);
    }
}
#endif
