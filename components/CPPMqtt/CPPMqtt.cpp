#include "CPPMqtt.h"
#include "TaskNotifier.h"

const char *MqttGeneral::TAG = "MQTT_feeder";
esp_mqtt_client_handle_t MqttGeneral::client = esp_mqtt_client_init(&MqttGeneral::mqtt_cfg_client);
MqttGeneral::_config_message_t client_message;
esp_mqtt_client_config_t MqttGeneral::mqtt_cfg_client{};
QueueHandle_t MqttGeneral::messageQueue = nullptr;

MqttGeneral::MQTT_t MqttGeneral::mqttBuffeeders = {
    .taskHandle = xTaskGetCurrentTaskHandle()};

TaskHandle_t MqttGeneral::getTaskToCaptorRight()
{
    return taskToCaptorRight; // Retourne le handle de la tâche associée au capteur droit
}

TaskHandle_t MqttGeneral::getTaskToCaptorLeft()
{
    return taskToCaptorLeft; // Retourne le handle de la tâche associée au capteur gauche
}

// Définit les informations de connexion au broker MQTT
void MqttGeneral::SetCredentials(const char *hostname, const char *username,
                                 const char *client_id, const char *testament_topic, const char *password)
{ // ip_perso=&hostname; testament "souris_city/01/SC01F01/$connected"
    const esp_mqtt_client_config_t new_mqtt_cfg_client = {
        .broker = {
            .address = {.hostname = hostname,
                        .transport = MQTT_TRANSPORT_OVER_TCP,
                        .port = 1883}},
        .credentials = {.username = username, .client_id = client_id, .authentication = {.password = password}},
        .session = {.last_will = {.topic = testament_topic, .msg = "false"}, .keepalive = 10, .protocol_ver = MQTT_PROTOCOL_V_3_1},
        .network = {.reconnect_timeout_ms = 5000}};
    esp_mqtt_set_config(client, &new_mqtt_cfg_client);

       // Mise à jour de la configuration existante
    esp_err_t err = esp_mqtt_set_config(MqttGeneral::client, &new_mqtt_cfg_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de la mise à jour de la configuration MQTT avec SetCredentials (%s)", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Configuration MQTT mise à jour avec succès par SetCredentials");
    }
}
Config GlobalConfig_topic;
Config& MqttGeneral::Config_topic = GlobalConfig_topic;

ConfigTmaze GlobalConfigtm_topic;
ConfigTmaze& MqttGeneral::Configtm_topic = GlobalConfigtm_topic;

MqttEventHandlers GlobalEventHandlers;
MqttEventHandlers& MqttGeneral::EventHandlers = GlobalEventHandlers;

MqttGeneral::MqttGeneral(TaskHandle_t taskToNotify, QueueHandle_t commandQueuemotor, Config &config_topic, MqttEventHandlers &EventHandlers)
    : taskHandle(nullptr), taskToNotify(taskToNotify), commandQueuemotor(commandQueuemotor)
{       MqttGeneral::Config_topic = config_topic;

     MqttGeneral::EventHandlers = EventHandlers;
    messageQueue = xQueueCreate(10, sizeof(MqttEventHandlers::MqttMessage)); // Taille de la file d'attente et taille de chaque message
}

MqttGeneral::MqttGeneral(TaskHandle_t taskToNotify, QueueHandle_t commandQueuemotor, ConfigTmaze &configtm_topic, MqttEventHandlers &EventHandlers)
    : taskHandle(nullptr), taskToNotify(taskToNotify), commandQueuemotor(commandQueuemotor)
{    
    MqttGeneral::Configtm_topic = configtm_topic;
    MqttGeneral::EventHandlers = EventHandlers;

    messageQueue = xQueueCreate(10, sizeof(MqttEventHandlers::MqttMessage)); // Taille de la file d'attente et taille de chaque message
}

MqttGeneral::MqttGeneral(QueueHandle_t commandQueuemotor,ConfigTmaze &configtm_topic, MqttEventHandlers &EventHandlers)
: commandQueuemotor(commandQueuemotor)
{       MqttGeneral::Configtm_topic = configtm_topic;

     MqttGeneral::EventHandlers = EventHandlers;
    messageQueue = xQueueCreate(10, sizeof(MqttEventHandlers::MqttMessage)); // Taille de la file d'attente et taille de chaque message
}

MqttGeneral::~MqttGeneral(void)
{
    if (taskHandle != nullptr)
    {
        vTaskDelete(taskHandle);
    }
    if (messageQueue != nullptr)
    {
        vQueueDelete(messageQueue);
    }
}

void MqttGeneral::log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void MqttGeneral::setMotorTaskHandles(TaskHandle_t motorRightHandle, TaskHandle_t motorLeftHandle)
{
    taskToNotifyMotorRight = motorRightHandle;
    taskToNotifyMotorLeft = motorLeftHandle;
}
// Fonction principale qui exécute la tâche MQTT
void MqttGeneral::run(void *param)
{
    auto *mqttGeneral = static_cast<MqttGeneral *>(param);
    mqttGeneral->execute();
}

// Exécution de la logique de traitement des messages MQTT
void MqttGeneral::execute()
{
    MqttEventHandlers::MqttMessage msg;
    while (true)
    {
        ESP_LOGI(TAG, "wait MQTT message");

        if (xQueueReceive(messageQueue, &msg, portMAX_DELAY))
        {
            // Traiter le message reçu
            ESP_LOGI(TAG, "Received queue message: topic='%s', data='%s'", msg.topic, msg.data);

            // Appliquer ici la logique de filtrage des messages
            if (std::string(msg.data) == "ON")
            {
                // command = 1; // Déterminez la commande basée sur le sujet et les données
                TaskNotifier::notify(taskToNotify, 1); //
                ESP_LOGI(TAG, " Notification envoyé au moteur : ");
            }
            else if (std::string(msg.data) == "EntrerRight")
            {
                TaskNotifier::notify(taskToNotifyMotorRight, 1); //
                ESP_LOGI(TAG, "Notififier pour servo entrer");
            }
            else if (std::string(msg.data) == "SortirRight")
            {
                TaskNotifier::notify(taskToNotifyMotorRight, 2); //
                ESP_LOGI(TAG, "Notififier pour servo sortir");
            }
            else if (std::string(msg.data) == "Entrerleft")
            {
                TaskNotifier::notify(taskToNotifyMotorLeft, 1); //
                ESP_LOGI(TAG, "Notififier pour servo entrer");
            }
            else if (std::string(msg.data) == "Sortirleft")
            {
                TaskNotifier::notify(taskToNotifyMotorLeft, 2); //
                ESP_LOGI(TAG, "Notififier pour servo sortir");
            }
            
        }
    }
}

void MqttGeneral::startTask(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority)
{
    xTaskCreate(&MqttGeneral::run, taskName, stackSize, this, taskPriority, &taskHandle);
}

void MqttGeneral::runcapteursR(void *param)
{
    auto *mqttGeneral = static_cast<MqttGeneral *>(param);
    mqttGeneral->executecapteursR();
}

void MqttGeneral::runcapteursL(void *param)
{
    auto *mqttGeneral = static_cast<MqttGeneral *>(param);
    mqttGeneral->executecapteursL();
}
void MqttGeneral::executecapteursR()
{
    while (true)
    {
    int ulNotificationValueRight =0;
        bool state;
        //ESP_LOGI(TAG, "Attendre la notification d'un capteur");

        // Attendre les notifications pour les capteurs gauche et droit
         ulNotificationValueRight = TaskNotifier::waitForNotification(taskToCaptorRight,pdMS_TO_TICKS(10));

        // Traiter la notification du capteur droit
        if (ulNotificationValueRight != 0)

        {
            ESP_LOGI(TAG, "value notification pour capteur droit: %d", ulNotificationValueRight);
            switch (ulNotificationValueRight)
            {
            case 4: // Notification pour un événement "Nose poke" du capteur droit
                ESP_LOGI(TAG, "Nose poke détecté sur capteur droit");
                state = 1;
                MqttGeneral::EventHandlers.MQTT_publish_beamRight(client, state, Configtm_topic);
                break;
            case 5: // Notification pour retour à la position du capteur droit
                ESP_LOGI(TAG, "Retour à la position détecté sur capteur droit");
                state = 0;
                MqttGeneral::EventHandlers.MQTT_publish_nose_poke(client, state, Config_topic);
                break;
            default:
                ESP_LOGI(TAG, "Notification inconnue pour capteur droit: %d", ulNotificationValueRight);
                break;
            }
        }
        
    }
}

void MqttGeneral::executecapteursL()
{
    while (true)
    {int ulNotificationValueLeft =0;
    
        bool state;
        //ESP_LOGI(TAG, "Attendre la notification d'un capteur");

        // Attendre les notifications pour les capteurs gauche et droit
        ulNotificationValueLeft = TaskNotifier::waitForNotification(taskToCaptorLeft,pdMS_TO_TICKS(10));

        // Traiter la notification du capteur gauche
        if (ulNotificationValueLeft != 0)
        {
            ESP_LOGI(TAG, "value notification pour capteur gauche: %d", ulNotificationValueLeft);
            switch (ulNotificationValueLeft)
            {
            case 4: // Notification pour un événement "Nose poke" du capteur gauche
                ESP_LOGI(TAG, "Nose poke détecté sur capteur gauche");
                state = 1;
                MqttGeneral::EventHandlers.MQTT_publish_beamLeft(client, state, Configtm_topic);
                break;
            case 5: // Notification pour retour à la position du capteur gauche
                ESP_LOGI(TAG, "Retour à la position détecté sur capteur gauche");
                state = 0;
                MqttGeneral::EventHandlers.MQTT_publish_nose_poke(client, state, Config_topic);
                break;
            default:
                ESP_LOGI(TAG, "Notification inconnue pour capteur gauche: %d", ulNotificationValueLeft);
                break;
            }
        }
    }
}


void MqttGeneral::startTaskcapteursRight(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority)
{
    xTaskCreate(&MqttGeneral::runcapteursR,taskName, stackSize, this,taskPriority, &taskToCaptorRight); // Tâche pour capteur droit
    ESP_LOGI("thread mqtt capteur droit", "La tâche pour le capteur droit a été créée. Num: %p", taskToCaptorRight);
}

void MqttGeneral::startTaskcapteursLeft(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority)
{
    xTaskCreate(&MqttGeneral::runcapteursL,taskName, stackSize, this,taskPriority, &taskToCaptorLeft); // Tâche pour capteur gauche
    ESP_LOGI("thread mqtt capteur gauche", "La tâche pour le capteur gauche a été créée. Num: %p", taskToCaptorLeft);
}

void MqttGeneral::mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    //   if (strcmp(Config_topic.getReponses(), "1") == 0) {
    //     ESP_LOGI(TAG, "Using Config");
     //        EventHandlers.handleMqttEventConnected(client, Config_topic);
    //     } else if (strcmp(Configtm_topic.getReponses(), "1") == 0) {
    //                 ESP_LOGI(TAG, "Using ConfigTmaze");

             EventHandlers.handleMqttEventConnectedTmaze(client, Configtm_topic);
    //     } else {
    //         ESP_LOGE(TAG, "ConfigTmaze is null");
    //     }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        MqttEventHandlers::processMqttEventMessage(event, messageQueue);

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void MqttGeneral::mqtt_app_start()
{
    esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_SUBSCRIBED, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_UNSUBSCRIBED, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_PUBLISHED, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_BEFORE_CONNECT, mqtt_event_handler, client);

    esp_mqtt_client_start(client);
}

void MqttGeneral::runmotor(void *param)
{
    auto *mqttGeneral = static_cast<MqttGeneral *>(param);
    mqttGeneral->executemotor();
}

void MqttGeneral::executemotor()
{
    int command = 0;
    int16_t nb_pellet_delivered = 0;
    while (true)
    { // bool state;
      //  ESP_LOGI(TAG, "Attendre la command de fin de séquence du motor");

        if (xQueueReceive(commandQueuemotor, &command, portMAX_DELAY) == pdTRUE)
        {
            // ESP_LOGI("mqtt MotorTask", "Reçu une commande: %d", command);

            switch (command)
            {
            case 6: //
                nb_pellet_delivered++;
                ESP_LOGI(TAG, "nb publish_delivered : %d", nb_pellet_delivered);
                MqttGeneral::EventHandlers.MQTT_publish_pellet_delivered(client, Config_topic);
                break;
            case 7: //
                ESP_LOGI(TAG, "Retour à la position");
                break;
            default:
                ESP_LOGI(TAG, "Notification inconnue pour capteur: %d", command);
                break;
            }
        }
    }
}

void MqttGeneral::startTaskmotor(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority)
{
    xTaskCreate(&MqttGeneral::runmotor,
                taskName, stackSize, this,
                taskPriority, NULL);
}
