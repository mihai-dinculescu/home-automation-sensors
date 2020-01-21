#include <bsec.h>
#include "bsec_serialized_configurations_iaq.h"

#include "config.h"
#include "MAD_ESP32.h"

#include "messaging.h"

#ifdef CAPABILITIES_SD
    #include "logging.h"
#endif

#ifdef CAPABILITIES_DISPLAY
    #include "display.h"
#endif

#ifdef CAPABILITIES_MOISTURE_SENSOR
    #include "moisture_sensor.h"
#endif

#ifdef CAPABILITIES_MQTT_CONFIG
    #include "mqtt_config.h"
    MQTTConfig *mqtt_config;
#endif

RTC_DATA_ATTR uint8_t sensor_state[BSEC_MAX_STATE_BLOB_SIZE] = {0};
RTC_DATA_ATTR int64_t sensor_state_time = 0;

Bsec sensor;

bool CheckSensor() {
    if (sensor.status < BSEC_OK) {
        LOGLN("BSEC error, status %d!", sensor.status);
        return false;
    } else if (sensor.status > BSEC_OK) {
        LOGLN("BSEC warning, status %d!", sensor.status);
    }

    if (sensor.bme680Status < BME680_OK) {
        LOGLN("Sensor error, bme680_status %d!", sensor.bme680Status);
        return false;
    } else if (sensor.bme680Status > BME680_OK) {
        LOGLN("Sensor warning, status %d!", sensor.bme680Status);
    }

    return true;
}

#ifdef BSEC_DUNP_STATE
void DumpState(const char *name, const uint8_t *state)
{
    LOGLN("%s:", name);

    for (int i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
        LOG("%02x ", state[i]);
        if (i % 16 == 15) {
            LOG("\n");
        }
    }

    LOG("\n");
}
#endif

#ifdef CAPABILITIES_MQTT_CONFIG
void MessageReceived(String &topic, String &payload) {
    mqtt_config->HandleMessageReceived(payload);
}
#endif

void SetupBsec()
{
    Wire.begin(board.pins.I2C_SDA, board.pins.I2C_SCL);

    sensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
    if (!CheckSensor()) {
        LOGLN("Failed to init BME680, check wiring!");
        board.FatalError();
    }
    LOGLN("BSEC version %d.%d.%d.%d.", sensor.version.major, sensor.version.minor, sensor.version.major_bugfix, sensor.version.minor_bugfix);

    sensor.setConfig(bsec_config_iaq);
    if (!CheckSensor()) {
        board.FatalError();
    }

    if (sensor_state_time) {
        #ifdef BSEC_DUNP_STATE
            DumpState("retrieveState", sensor_state);
        #endif
        sensor.setState(sensor_state);

        if (!CheckSensor()) {
            board.FatalError();
        } else {
            LOGLN("Successfully set state from %lld.", sensor_state_time);
        }
    } else {
        LOGLN("Saved state missing!");

        #ifdef CAPABILITIES_MQTT_CONFIG
            mqtt_config = new MQTTConfig(BSEC_MAX_STATE_BLOB_SIZE, sensor_state);
            mqtt_config->ReceiveConfigMessage(MessageReceived);

            if (mqtt_config->received_status) {
                #ifdef BSEC_DUNP_STATE
                    DumpState("retrieveStateSD", sensor_state);
                #endif
                sensor.setState(sensor_state);

                if (!CheckSensor()) {
                    board.FatalError();
                } else {
                    LOGLN("Successfully set state from MQTT.");
                }
            } else {
                LOGLN("Failed to fetch state from MQTT!");
            }
        #endif

        #ifdef CAPABILITIES_SD
            if (DataRead(BSEC_MAX_STATE_BLOB_SIZE, sensor_state)) {
                #ifdef BSEC_DUNP_STATE
                    DumpState("retrieveStateSD", sensor_state);
                #endif
                sensor.setState(sensor_state);

                if (!CheckSensor()) {
                    #ifdef CAPABILITIES_SD
                        DataDelete();
                        LogRestart("Invalid BSEC data. Deleted.");
                    #endif

                    board.DeepSleep(10);
                } else {
                    LOGLN("Successfully set state from file.");
                }
            } else {
                LOGLN("Saved state SD missing!");
            }
        #endif
    }

    bsec_virtual_sensor_t sensor_list[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_STATIC_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    };

    sensor.updateSubscription(sensor_list, sizeof(sensor_list) / sizeof(sensor_list[0]), BSEC_SAMPLE_RATE_ULP);
    if (!CheckSensor()) {
        LOGLN("Failed to update subscription!");
        board.FatalError();
    }

    LOGLN("BSEC sensor init done.");
}

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

        sensor_state_time = board.GetTimestamp();
        sensor.getState(sensor_state);
        #ifdef BSEC_DUNP_STATE
            DumpState("saveState", sensor_state);
        #endif
        LOGLN("Saved state to RTC memory at %lld", sensor_state_time);

        #ifdef CAPABILITIES_MQTT_CONFIG
            if (mqtt_config->SendConfigMessage()) {
                LOGLN("Saved state to MQTT.");
            } else {
                LOGLN("Failed to save state to MQTT. Reason %d!", mqtt_client.lastError());
                board.DeepSleep(10);
            }
        #endif

        #ifdef CAPABILITIES_SD
            DataWrite(BSEC_MAX_STATE_BLOB_SIZE, sensor_state);
            LOGLN("Saved state to file.");
        #endif

        LogData(sensor.temperature, sensor.humidity, sensor.pressure, sensor.iaq, sensor.iaqAccuracy, plant_moisture);

        #ifdef CAPABILITIES_DISPLAY
            DisplayData(sensor.temperature, sensor.humidity, sensor.pressure, sensor.iaq, sensor.iaqAccuracy, plant_moisture);
        #endif

        board.DeepSleep(5 * 60);
    }
}
#endif
