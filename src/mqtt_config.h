#ifndef MQTT_CONFIG_H
    #define MQTT_CONFIG_H

    #include <stdint.h>
    #include <string>
    #include <sstream>
    #include <vector>
    #include <iterator>
    #include <functional>

    class MQTTConfig
    {
        public:
            uint16_t sensor_state_length;
            uint8_t *sensor_state;
            bool received_status;
            void ReceiveConfigMessage(MQTTClientCallbackSimple cb);
            void HandleMessageReceived(String &payload);
            bool SendConfigMessage();
            MQTTConfig(uint16_t sensor_state_length, uint8_t *sensor_state):
                sensor_state_length(sensor_state_length), sensor_state(sensor_state), received_status(false) {}
        private:
            MQTTConfig() {}
    };
#endif
