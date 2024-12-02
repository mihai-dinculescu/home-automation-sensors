#include "config.h"
#include "MAD_ESP32.h"
#include "storage.h"

#include "sensor_bsec.h"

const uint8_t _bsec_config_iaq[] = {
#include "config/bme680/bme680_iaq_33v_300s_28d/bsec_iaq.txt"
};

SensorBsec sensor_bsec;

RTC_DATA_ATTR uint8_t _sensor_state[BSEC_MAX_STATE_BLOB_SIZE] = {0};
RTC_DATA_ATTR int64_t _sensor_state_time = 0;

bool SensorBsec::CheckSensor()
{
    if (_sensor.status < BSEC_OK)
    {
        LOGLNT("BSEC error code : %s", String(_sensor.status));
        return false;
    }
    else if (_sensor.status > BSEC_OK)
    {
        LOGLNT("BSEC warning code : %s", String(_sensor.status));
    }

    if (_sensor.sensor.status < BME68X_OK)
    {
        LOGLNT("BME68X error code : %s", String(_sensor.sensor.status));
        return false;
    }
    else if (_sensor.sensor.status > BME68X_OK)
    {
        LOGLNT("BME68X warning code : %s", String(_sensor.sensor.status));
    }

    return true;
}

bool SensorBsec::Setup()
{
    Wire.begin(board.pins.I2C_SDA, board.pins.I2C_SCL);

    _sensor.begin(BME68X_I2C_ADDR_HIGH, Wire);
    if (!CheckSensor())
    {
        LOGLNT("Failed to init BME680, check wiring!");
        return false;
    }
    LOGLNT("BSEC version %d.%d.%d.%d.", _sensor.version.major, _sensor.version.minor, _sensor.version.major_bugfix, _sensor.version.minor_bugfix);

    _sensor.setConfig(_bsec_config_iaq);

    if (!CheckSensor())
    {
        LOGLNT("Invalid BSEC config!");
        return false;
    }

    if (_sensor_state_time)
    {
        _sensor.setState(_sensor_state);

        if (!CheckSensor())
        {
            LOGLNT("Invalid BSEC data after set state from RTC!");
            return false;
        }
        else
        {
            LOGLNT("Successfully set state from %lld.", _sensor_state_time);
        }
    }
    else
    {
        LOGLNT("Saved state missing in RTC.");

        if (storage.ConfigRead(BSEC_MAX_STATE_BLOB_SIZE, _sensor_state))
        {
            _sensor.setState(_sensor_state);

            if (!CheckSensor())
            {
                storage.ConfigDelete();
                const char *message = "Invalid BSEC data after set state from SD! Deleted.";
                LOGLNT("%s", message);
                storage.LogError(message);

                board.DeepSleep(10);
            }
            else
            {
                LOGLNT("Successfully set state from SD.");
            }
        }
        else
        {
            LOGLNT("Saved state missing in SD.");
        }
    }

    bsec_virtual_sensor_t sensor_list[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
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
        return false;
    }

    LOGLNT("BSEC sensor init done.");

    return true;
}

bool SensorBsec::SaveState()
{
    _sensor_state_time = board.GetTimestamp();
    _sensor.getState(_sensor_state);

    LOGLNT("Saved state to RTC memory at %lld.", _sensor_state_time);

    storage.ConfigWrite(BSEC_MAX_STATE_BLOB_SIZE, _sensor_state);
    LOGLNT("Saved state to file.");

    if (!CheckSensor())
    {
        LOGLNT("Invalid BSEC config after save state!");
        return false;
    }

    return true;
}

bool SensorBsec::Run(int64_t timeMilliseconds)
{
    if (_sensor.run(timeMilliseconds))
    {
        const bsecOutputs *outputs = _sensor.getOutputs();

        if (!outputs || !outputs->nOutputs)
        {
            return false;
        }

        LOGLNT("BSEC outputs:");
        LOGLNT("\t\tTime stamp = %d", (int)(outputs->output[0].time_stamp / INT64_C(1000000)));
        for (uint8_t i = 0; i < outputs->nOutputs; i++)
        {
            const bsecData output = outputs->output[i];
            switch (output.sensor_id)
            {
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                LOGLNT("\t\tTemperature = %f", output.signal);
                _raw_temperature = output.signal;
                break;
            case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
                LOGLNT("\t\tCompensated temperature = %f", output.signal);
                _temperature = output.signal;
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                LOGLNT("\t\tHumidity = %f", output.signal);
                _raw_humidity = output.signal;
                break;
            case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
                LOGLNT("\t\tCompensated humidity = %f", output.signal);
                _humidity = output.signal;
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                LOGLNT("\t\tPressure = %f", output.signal);
                _raw_pressure = output.signal;
                break;
            case BSEC_OUTPUT_IAQ:
                LOGLNT("\t\tIAQ = %f", output.signal);
                LOGLNT("\t\tIAQ accuracy = %d", (int)output.accuracy);
                _iaq = output.signal;
                _iaq_accuracy = output.accuracy;
                break;
            case BSEC_OUTPUT_STATIC_IAQ:
                LOGLNT("\t\tStatic IAQ = %f", output.signal);
                LOGLNT("\t\tStatic IAQ accuracy = %d", (int)output.accuracy);
                _static_iaq = output.signal;
                _static_iaq_accuracy = output.accuracy;
                break;
            case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
                LOGLNT("\t\tbVOC equivalent = %f", output.signal);
                LOGLNT("\t\tbVOC accuracy = %d", (int)output.accuracy);
                _breath_voc_equivalent = output.signal;
                _breath_voc_accuracy = output.accuracy;
                break;
            case BSEC_OUTPUT_CO2_EQUIVALENT:
                LOGLNT("\t\tCO2 equivalent = %f", output.signal);
                LOGLNT("\t\tCO2 accuracy = %d", (int)output.accuracy);
                _co2_equivalent = output.signal;
                _co2_accuracy = output.accuracy;
                break;
            default:
                break;
            }
        }

        return true;
    }

    return false;
}

float SensorBsec::getRawTemperature()
{
    return _raw_temperature;
}

float SensorBsec::getTemperature()
{
    return _temperature;
}

float SensorBsec::getRawHumidity()
{
    return _raw_humidity;
}

float SensorBsec::getHumidity()
{
    return _humidity;
}

float SensorBsec::getRawPressure()
{
    return _raw_pressure;
}

float SensorBsec::getIaq()
{
    return _iaq;
}

uint16_t SensorBsec::getIaqAccuracy()
{
    return _iaq_accuracy;
}

float SensorBsec::getStaticIaq()
{
    return _static_iaq;
}

uint16_t SensorBsec::getStaticIaqAccuracy()
{
    return _static_iaq_accuracy;
}

float SensorBsec::getBreathVocEquivalent()
{
    return _breath_voc_equivalent;
}

uint16_t SensorBsec::getBreathVocAccuracy()
{
    return _breath_voc_accuracy;
}

float SensorBsec::getCo2Equivalent()
{
    return _co2_equivalent;
}

uint16_t SensorBsec::getCo2Accuracy()
{
    return _co2_accuracy;
}

int64_t SensorBsec::getNextCall()
{
    return _sensor.getNextCall() / 1000;
}
