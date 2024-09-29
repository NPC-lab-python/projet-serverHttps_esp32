#include "MqttEventHandlers.h"
#include "Configurations.h"

MqttEventHandlers::MqttEventHandlers() {}
MqttEventHandlers::~MqttEventHandlers() {}
const char *TAG = "MqttEventHandlers";

// Implémentation de la fonction handleMqttEventConnected
void MqttEventHandlers::handleMqttEventConnected(esp_mqtt_client_handle_t client, const Config &Config_topic)
{
    /*esp_mqtt_client_subscribe(client, Config_topic.setter, 1);
    esp_mqtt_client_publish(client, Config_topic.connecte, "True", 0, 1, 1);
    esp_mqtt_client_publish(client, Config_topic.meta_device, "{\"type\": \"feeders\", \"location\": \"LMT\",\"properties\":[{\"name\": \"activate\", \"datatype\":\"str\", \"settable\": true, \"retain\": false},{\"name\":\"pellet_delivered\", \"datatype\": \"str\", \"settable\": false, \"retain\": false},{\"name\":\"beam_pellet\", \"datatype\": \"str\", \"settable\": false, \"retain\": false},{\"name\":\"nose_poke\", \"datatype\": \"str\", \"settable\": false, \"retain\": false}]}", 0, 1, 1);
*/
    if (client == nullptr)
    {
        ESP_LOGE(TAG, "MQTT client is null");
        return;
    }
    esp_mqtt_client_subscribe(client, Config_topic.getSetter(), 1);
    esp_mqtt_client_publish(client, Config_topic.getConnecte(), "True", 0, 1, 1);
    esp_mqtt_client_publish(client, Config_topic.getMetaDevice(), "{\"type\": \"feeders\", \"location\": \"LMT\",\"properties\":[{\"name\": \"activate\", \"datatype\":\"str\", \"settable\": true, \"retain\": false},{\"name\":\"pellet_delivered\", \"datatype\": \"str\", \"settable\": false, \"retain\": false},{\"name\":\"beam_pellet\", \"datatype\": \"str\", \"settable\": false, \"retain\": false},{\"name\":\"nose_poke\", \"datatype\": \"str\", \"settable\": false, \"retain\": false}]}", 0, 1, 1);
}
// Implémentation de la fonction handleMqttEventConnected
void MqttEventHandlers::handleMqttEventConnectedTmaze(esp_mqtt_client_handle_t client, const ConfigTmaze &Configtm_topic)
{
    if (client == nullptr)
    {
        ESP_LOGE(TAG, "MQTT client is null");
        return;
    }

    // Vérifier la validité des chaînes de caractères avant de les utiliser
    const char *setter = Configtm_topic.getSetter();
    const char *connecte = Configtm_topic.getConnecte();
    const char *metaServoRight = Configtm_topic.getMetaServoRight();
    const char *propertyServoRight = Configtm_topic.getPropertyServoRight();
    const char *metaServoLeft = Configtm_topic.getMetaServoLeft();
    const char *propertyServoLeft = Configtm_topic.getPropertyServoLeft();

    if (setter == nullptr || connecte == nullptr || metaServoRight == nullptr || propertyServoRight == nullptr)
    {
        ESP_LOGE(TAG, "Invalid topic or property detected in ConfigTmaze");
        return;
    }

    // Ajouter des logs pour vérifier les valeurs des topics
    ESP_LOGI(TAG, "Subscribing to topic: %s", setter);
    esp_mqtt_client_subscribe(client, setter, 1);

    ESP_LOGI(TAG, "Publishing to topic: %s", connecte);
    esp_mqtt_client_publish(client, connecte, "True", 0, 1, 1);

    ESP_LOGI(TAG, "Publishing servo right meta: %s with property: %s", metaServoRight, propertyServoRight);
    esp_mqtt_client_publish(client, metaServoRight, propertyServoRight, 0, 1, 1);

    ESP_LOGI(TAG, "Publishing servo left meta: %s with property: %s", metaServoLeft, propertyServoLeft);
    esp_mqtt_client_publish(client, metaServoLeft, propertyServoLeft, 0, 1, 1);
}
void MqttEventHandlers::processMqttEventMessage(esp_mqtt_event_handle_t event, QueueHandle_t messageQueue)
{
    MqttMessage msg;
    snprintf(msg.topic, sizeof(msg.topic), "%.*s", event->topic_len, event->topic);
    snprintf(msg.data, sizeof(msg.data), "%.*s", event->data_len, event->data);
    ESP_LOGI("MQTT", "Received topic: %s, data: %s", msg.topic, msg.data);

    // Appliquer ici la logique de filtrage des messages
    if (std::string(msg.data) == "ON")
    {
        xQueueSendToBack(messageQueue, &msg, portMAX_DELAY);
    }
    else if (std::string(msg.data) == "2")
    {
        xQueueSendToBack(messageQueue, &msg, portMAX_DELAY);
    }
    else if (std::string(msg.data) == "EntrerRight")
    {
        xQueueSendToBack(messageQueue, &msg, portMAX_DELAY);
    }
    else if (std::string(msg.data) == "SortirRight")
    {
        xQueueSendToBack(messageQueue, &msg, portMAX_DELAY);
    }
    else if (std::string(msg.data) == "Entrerleft")
    {
        xQueueSendToBack(messageQueue, &msg, portMAX_DELAY);
    }
    else if (std::string(msg.data) == "Sortirleft")
    {
        xQueueSendToBack(messageQueue, &msg, portMAX_DELAY);
    }
    std::string data = msg.data;

    // Gestion du changement d'ID du moteur
    if (data.rfind("change_id:", 0) == 0) { // Si le message commence par "change_id:"
        // Extraction des ID actuels et nouveaux
        int current_id = std::stoi(data.substr(10, 1)); // ID actuel à l'indice 10
        int new_id = std::stoi(data.substr(12, 1)); // Nouvel ID à l'indice 12

        // Créer une instance temporaire de ServoSCS15 pour changer l'ID
        ESP_LOGI("MQTT", "Changing motor ID from %d to %d", current_id, new_id);
        ServoSCS15* servo = new ServoSCS15(); // Allouer dynamiquement une instance

        // Changer l'ID et sauvegarder
        servo->change_motor_id(current_id, new_id);
        servo->save_servo_settings(new_id); // Sauvegarder le nouvel ID dans l'EEPROM

        // Publier un message MQTT pour confirmer le changement
        esp_mqtt_client_publish(event->client, "servo/id_changed", "ID changed successfully", 0, 1, 0);

        // Libérer l'instance du servo après le changement d'ID
        delete servo; // Supprimer l'instance pour libérer la mémoire
        ESP_LOGI("MQTT", "Servo instance deleted after ID change");
    }
}

void MqttEventHandlers::MQTT_publish(esp_mqtt_client_handle_t client, const Config &Config_topic)
{
    esp_mqtt_client_publish(client, Config_topic.getSetter(), "Bien reçu", 0, 1, 1);
}

void MqttEventHandlers::MQTT_publish_pellet_delivered(esp_mqtt_client_handle_t client, const Config &Config_topic)
{
    esp_mqtt_client_publish(client, Config_topic.getPelletDelivered(), "FIN MOTOR", 0, 1, 0);
}

void MqttEventHandlers::MQTT_publish_beamRight(esp_mqtt_client_handle_t client, bool state, const ConfigTmaze &Configtm_topic)
{
    if (state)
    {
        ESP_LOGI("mqtt beam right", "true");
        esp_mqtt_client_publish(client, Configtm_topic.getBeamRight(), "broken", 0, 1, 0);
    }
}

void MqttEventHandlers::MQTT_publish_beamLeft(esp_mqtt_client_handle_t client, bool state, const ConfigTmaze &Configtm_topic)
{
    if (state)
    {
        ESP_LOGI("mqtt beam left", "true");
        esp_mqtt_client_publish(client, Configtm_topic.getBeamLeft(), "broken", 0, 1, 0);
    }
}

void MqttEventHandlers::MQTT_publish_nose_poke(esp_mqtt_client_handle_t client, bool state, const Config &Config_topic)
{
    if (state)
    {
        ESP_LOGI("mqtt nosepoke", "true");
        esp_mqtt_client_publish(client, Config_topic.getBeamNosePoke(), "out", 0, 1, 0);
    }
    else
    {
        esp_mqtt_client_publish(client, Config_topic.getBeamNosePoke(), "in", 0, 1, 0);
    }
}

void MqttEventHandlers::MQTT_publish_pellet(esp_mqtt_client_handle_t client, bool state, const Config &Config_topic)
{
    if (state)
    {
        ESP_LOGI("mqtt beam pellet", "true");
        esp_mqtt_client_publish(client, Config_topic.getBeamPellet(), "true", 0, 1, 0);
    }
    else
    {
        esp_mqtt_client_publish(client, Config_topic.getBeamPellet(), "false", 0, 1, 0);
    }
}
