/**
 * Bsec2Ext::run is derived from Bsec2::run in the Bosch Sensortec BSEC2 Arduino
 * library (bsec2.cpp, v1.10.2610), which is licensed as follows:
 *
 * Copyright (c) 2021 Bosch Sensortec GmbH. All rights reserved.
 *
 * BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "bsec2_ext.h"

// Copy of Bsec2::run with the timestamp provided by the caller.
bool Bsec2Ext::run(int64_t timeMilliseconds)
{
    uint8_t nFieldsLeft = 0;
    bme68xData data;
    int64_t currTimeNs = timeMilliseconds * INT64_C(1000000);
    opMode = bmeConf.op_mode;

    if (currTimeNs >= bmeConf.next_call)
    {
        /* Provides the information about the current sensor configuration that is
           necessary to fulfill the input requirements, eg: operation mode, timestamp
           at which the sensor data shall be fetched etc */
        status = bsec_sensor_control_m(bsecInstance, currTimeNs, &bmeConf);
        if (status != BSEC_OK)
            return false;

        switch (bmeConf.op_mode)
        {
        case BME68X_FORCED_MODE:
            setBme68xConfigForced();
            break;
        case BME68X_PARALLEL_MODE:
            if (opMode != bmeConf.op_mode)
            {
                setBme68xConfigParallel();
            }
            break;

        case BME68X_SLEEP_MODE:
            if (opMode != bmeConf.op_mode)
            {
                sensor.setOpMode(BME68X_SLEEP_MODE);
                opMode = BME68X_SLEEP_MODE;
            }
            break;
        }

        if (sensor.checkStatus() == BME68X_ERROR)
            return false;

        if (bmeConf.trigger_measurement && bmeConf.op_mode != BME68X_SLEEP_MODE)
        {
            if (sensor.fetchData())
            {
                do
                {
                    nFieldsLeft = sensor.getData(data);
                    /* check for valid gas data */
                    if (data.status & BME68X_GASM_VALID_MSK)
                    {
                        /* Convert sensor raw pressure unit from pascal to hecto pascal */
                        data.pressure *= 0.01f;

                        if (!processData(currTimeNs, data))
                        {
                            return false;
                        }
                    }
                } while (nFieldsLeft);
            }
        }
    }
    return true;
}

int64_t Bsec2Ext::getNextCall(void)
{
    return bmeConf.next_call / 1000;
}
