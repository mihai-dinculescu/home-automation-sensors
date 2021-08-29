#include "config.h"
#include "MAD_ESP32.h"
#include "storage.h"

#include "sensor_bsec.h"

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
const uint8_t _bsec_config_iaq[] = {
#include "config/generic_33v_300s_28d/bsec_iaq.txt"
};

SensorBsec sensor_bsec;

RTC_DATA_ATTR uint8_t _sensor_state[BSEC_MAX_STATE_BLOB_SIZE] = {0};
RTC_DATA_ATTR int64_t _sensor_state_time = 0;

bool SensorBsec::CheckSensor()
{
    if (_sensor.status < BSEC_OK)
    {
        LOGLNT("BSEC error, status %d!", _sensor.status);
        return false;
    }
    else if (_sensor.status > BSEC_OK)
    {
        LOGLNT("BSEC warning, status %d!", _sensor.status);
    }

    if (_sensor.bme680Status < BME680_OK)
    {
        LOGLNT("Sensor error, bme680_status %d!", _sensor.bme680Status);
        return false;
    }
    else if (_sensor.bme680Status > BME680_OK)
    {
        LOGLNT("Sensor warning, status %d!", _sensor.bme680Status);
    }

    return true;
}

#ifdef BSEC_DUMP_STATE
void SensorBsec::DumpState(const char *name, const uint8_t *state)
{
    LOGLNT("%s:", name);

    for (int i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++)
    {
        LOG("%02x ", state[i]);
        if (i % 16 == 15)
        {
            LOG("\n");
        }
    }

    LOG("\n");
}
#endif

void SensorBsec::Setup()
{
    Wire.begin(board.pins.I2C_SDA, board.pins.I2C_SCL);

    _sensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
    if (!CheckSensor())
    {
        LOGLNT("Failed to init BME680, check wiring!");
        board.FatalError();
    }
    LOGLNT("BSEC version %d.%d.%d.%d.", _sensor.version.major, _sensor.version.minor, _sensor.version.major_bugfix, _sensor.version.minor_bugfix);

    _sensor.setConfig(_bsec_config_iaq);

    if (!CheckSensor())
    {
        LOGLNT("Invalid BSEC config!");
        board.FatalError();
    }

    if (_sensor_state_time)
    {
#ifdef BSEC_DUMP_STATE
        DumpState("retrieveState", _sensor_state);
#endif
        _sensor.setState(_sensor_state);

        if (!CheckSensor())
        {
            board.FatalError();
        }
        else
        {
            LOGLNT("Successfully set state from %lld.", _sensor_state_time);
        }
    }
    else
    {
        LOGLNT("Saved state missing!");

        if (storage.ConfigRead(BSEC_MAX_STATE_BLOB_SIZE, _sensor_state))
        {
#ifdef BSEC_DUMP_STATE
            DumpState("retrieveStateSD", _sensor_state);
#endif
            _sensor.setState(_sensor_state);

            if (!CheckSensor())
            {
                storage.ConfigDelete();
                storage.LogError("Invalid BSEC data. Deleted.");

                board.DeepSleep(10);
            }
            else
            {
                LOGLNT("Successfully set state from file.");
            }
        }
        else
        {
            LOGLNT("Saved state SD missing!");
        }
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

    _sensor.updateSubscription(sensor_list, sizeof(sensor_list) / sizeof(sensor_list[0]), BSEC_SAMPLE_RATE_ULP);

    if (!CheckSensor())
    {
        LOGLNT("Failed to update subscription!");
        board.FatalError();
    }

    LOGLNT("BSEC sensor init done.");
}

void SensorBsec::SaveState()
{
    _sensor_state_time = board.GetTimestamp();
    _sensor.getState(_sensor_state);

#ifdef BSEC_DUMP_STATE
    DumpState("saveState", _sensor_state);
#endif
    LOGLNT("Saved state to RTC memory at %lld", _sensor_state_time);

    storage.ConfigWrite(BSEC_MAX_STATE_BLOB_SIZE, _sensor_state);
    LOGLNT("Saved state to file.");

    if (!CheckSensor())
    {
        LOGLNT("Invalid BSEC config after save state!");
        board.FatalError();
    }
}

bool SensorBsec::Run(int64_t timeMilliseconds)
{
    return _sensor.run(timeMilliseconds);
}

float SensorBsec::getRawTemperature()
{
    return _sensor.rawTemperature;
}

float SensorBsec::getTemperature()
{
    return _sensor.temperature;
}

float SensorBsec::getRawHumidity()
{
    return _sensor.rawHumidity;
}

float SensorBsec::getHumidity()
{
    return _sensor.humidity;
}

float SensorBsec::getPressure()
{
    return _sensor.pressure;
}

float SensorBsec::getIaq()
{
    return _sensor.iaq;
}

uint16_t SensorBsec::getIaqAccuracy()
{
    return _sensor.iaqAccuracy;
}

float SensorBsec::getStaticIaq()
{
    return _sensor.staticIaq;
}

uint16_t SensorBsec::getStaticIaqAccuracy()
{
    return _sensor.staticIaqAccuracy;
}

float SensorBsec::getBreathVocEquivalent()
{
    return _sensor.breathVocEquivalent;
}

uint16_t SensorBsec::getBreathVocAccuracy()
{
    return _sensor.breathVocAccuracy;
}

float SensorBsec::getCo2Equivalent()
{
    return _sensor.co2Equivalent;
}

uint16_t SensorBsec::getCo2Accuracy()
{
    return _sensor.co2Accuracy;
}

float SensorBsec::getGasResistance()
{
    return _sensor.gasResistance;
}

int64_t SensorBsec::getNextCall()
{
    return _sensor.nextCall;
}
