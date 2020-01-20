#ifndef MESSAGING_H
    #define MESSAGING_H

    #include <MQTT.h>

    extern MQTTClient mqtt_client;

    void SetupMQTT();
    void LogData(float temperature, float humidity, float pressure, float iaq_estimate, uint16_t iaq_accuracy, uint16_t plant_moisture);
#endif
