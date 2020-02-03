#include <bsec.h>

/* Configure the BSEC library with information about the sensor
    18v/33v = Voltage at Vdd. 1.8V or 3.3V
    3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
    4d/28d = Operating age of the sensor in days
    generic_18v_3s_4d
    generic_18v_3s_28d
    generic_18v_300s_4d
    generic_18v_300s_28d
    generic_33v_3s_4d
    generic_33v_3s_28d
    generic_33v_300s_4d
    generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_300s_28d/bsec_iaq.txt"
};

#include "config.h"
#include "MAD_ESP32.h"

#ifdef CAPABILITIES_SD
    #include "logging.h"
#endif

RTC_DATA_ATTR uint8_t sensor_state[BSEC_MAX_STATE_BLOB_SIZE] = {0};
RTC_DATA_ATTR int64_t sensor_state_time = 0;

Bsec sensor;

bool CheckSensor() {
    if (sensor.status < BSEC_OK) {
        LOGLNT("BSEC error, status %d!", sensor.status);
        return false;
    } else if (sensor.status > BSEC_OK) {
        LOGLNT("BSEC warning, status %d!", sensor.status);
    }

    if (sensor.bme680Status < BME680_OK) {
        LOGLNT("Sensor error, bme680_status %d!", sensor.bme680Status);
        return false;
    } else if (sensor.bme680Status > BME680_OK) {
        LOGLNT("Sensor warning, status %d!", sensor.bme680Status);
    }

    return true;
}

#ifdef BSEC_DUNP_STATE
void DumpState(const char *name, const uint8_t *state)
{
    LOGLNT("%s:", name);

    for (int i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
        LOG("%02x ", state[i]);
        if (i % 16 == 15) {
            LOG("\n");
        }
    }

    LOG("\n");
}
#endif

void SetupBsec()
{
    Wire.begin(board.pins.I2C_SDA, board.pins.I2C_SCL);

    sensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
    if (!CheckSensor()) {
        LOGLNT("Failed to init BME680, check wiring!");
        board.FatalError();
    }
    LOGLNT("BSEC version %d.%d.%d.%d.", sensor.version.major, sensor.version.minor, sensor.version.major_bugfix, sensor.version.minor_bugfix);

    sensor.setConfig(bsec_config_iaq);

    if (!CheckSensor()) {
        LOGLNT("Invalid BSEC config!");
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
            LOGLNT("Successfully set state from %lld.", sensor_state_time);
        }
    } else {
        LOGLNT("Saved state missing!");

        #ifdef CAPABILITIES_SD
            if (DataRead(BSEC_MAX_STATE_BLOB_SIZE, sensor_state)) {
                #ifdef BSEC_DUNP_STATE
                    DumpState("retrieveStateSD", sensor_state);
                #endif
                sensor.setState(sensor_state);

                if (!CheckSensor()) {
                    #ifdef CAPABILITIES_SD
                        DataDelete();
                        LogError("Invalid BSEC data. Deleted.");
                    #endif

                    board.DeepSleep(10);
                } else {
                    LOGLNT("Successfully set state from file.");
                }
            } else {
                LOGLNT("Saved state SD missing!");
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
        LOGLNT("Failed to update subscription!");
        board.FatalError();
    }

    LOGLNT("BSEC sensor init done.");
}

void SaveBsecState()
{
    sensor_state_time = board.GetTimestamp();
    sensor.getState(sensor_state);

    #ifdef BSEC_DUNP_STATE
        DumpState("saveState", sensor_state);
    #endif
    LOGLNT("Saved state to RTC memory at %lld", sensor_state_time);

    #ifdef CAPABILITIES_SD
        DataWrite(BSEC_MAX_STATE_BLOB_SIZE, sensor_state);
        LOGLNT("Saved state to file.");
    #endif

    if (!CheckSensor()) {
        LOGLNT("Invalid BSEC config after save state!");
        board.FatalError();
    }
}
