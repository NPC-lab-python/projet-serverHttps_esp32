#ifndef MQTTEVENTHANDLERS_H
#define MQTTEVENTHANDLERS_H

#include "mqtt_client.h" // Assurez-vous d'inclure les en-têtes nécessaires pour esp_mqtt_client_handle_t, etc.
#include "freertos/queue.h" // Pour QueueHandle_t
#include <iostream>
#include "esp_log.h" // Pour ESP_LOGI
#include "Configurations.h"
#include "ServoSCS15.h"


class MqttEventHandlers
{
public:

    MqttEventHandlers();
    ~MqttEventHandlers(void);

    void handleMqttEventConnected(esp_mqtt_client_handle_t client, const Config& Config_topic);
    void handleMqttEventConnectedTmaze(esp_mqtt_client_handle_t client, const ConfigTmaze& Configtm_topic);

    struct MqttMessage
    {
        char topic[128];
        char data[128];
    };

    static void processMqttEventMessage(esp_mqtt_event_handle_t event, QueueHandle_t messageQueue);

    void MQTT_publish(esp_mqtt_client_handle_t client, const Config& Config_topic);
    void MQTT_publish_pellet_delivered(esp_mqtt_client_handle_t client, const Config& Config_topic);
    void MQTT_publish_nose_poke(esp_mqtt_client_handle_t client,bool state, const Config& Config_topic);
    void MQTT_publish_pellet(esp_mqtt_client_handle_t client,bool state, const Config& Config_topic);
    void MQTT_publish_beamRight(esp_mqtt_client_handle_t client, bool state, const ConfigTmaze &Configtm_topic);
    void MQTT_publish_beamLeft(esp_mqtt_client_handle_t client, bool state, const ConfigTmaze &Configtm_topic);



};
#endif // MQTTEVENTHANDLERS_H