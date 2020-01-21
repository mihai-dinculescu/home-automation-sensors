#include "config.h"

#ifdef CAPABILITIES_MQTT_CONFIG
    #include "messaging.h"
    #include "mqtt_config.h"

    template <typename Out>
    void split(const std::string &s, char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
            *result++ = item;
        }
    }

    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }

    void MQTTConfig::HandleMessageReceived(String &payload)
    {
        mqtt_client.unsubscribe(config.mqtt_topic_memory);

        std::vector<std::string> values = split(payload.c_str(), ' ');

        if (sensor_state_length == values.size()) {
            for(int i = 0; i < sensor_state_length; i++) {
                sensor_state[i] = (uint8_t)atoi(values[i].c_str());
            }

            received_status = true;
        }
    }

    void MQTTConfig::ReceiveConfigMessage(MQTTClientCallbackSimple cb)
    {
        mqtt_client.onMessage(cb);
        mqtt_client.subscribe(config.mqtt_topic_memory);

        uint64_t lastMillis = millis();
        while (lastMillis + 10 * 1000 >= millis() && !received_status) {
            mqtt_client.loop();
        }
    }

    bool MQTTConfig::SendConfigMessage()
    {
        std::ostringstream messageStream;
        for (int i = 0; i < sensor_state_length; i++) {
            messageStream << (uint16_t)sensor_state[i] + 1;
            if (i < sensor_state_length - 1) {
                messageStream << ' ';
            }
        }

        return mqtt_client.publish(config.mqtt_topic_memory, messageStream.str().c_str(), true, 2);
    }
#endif
