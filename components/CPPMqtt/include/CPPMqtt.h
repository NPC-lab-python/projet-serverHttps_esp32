// #pragma once
#ifndef CPPMQTT_H
#define CPPMQTT_H

// #include "CPPGpio.hpp"
#include "mqtt_client.h"

#include <string.h>
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "TaskNotifier.h"
#include "Configurations.h"
#include "MqttEventHandlers.h"

class MqttGeneral
{
private:
    TaskHandle_t taskHandle; // Handle de la tâche MQTT
    TaskHandle_t taskToNotify; // Handle de la tâche à notifier

    TaskHandle_t taskToNotifyMotorRight;
    TaskHandle_t taskToNotifyMotorLeft;

    TaskHandle_t taskToCaptorRight; // Handle du capteur droit (right)
    TaskHandle_t taskToCaptorLeft;  // Handle du capteur gauche (left)

    static QueueHandle_t messageQueue; // File de messages pour recevoir les événements MQTT
    QueueHandle_t commandQueuemotor; // File de commandes pour le moteur

    static Config &Config_topic; // Configuration des topics MQTT
    static ConfigTmaze &Configtm_topic; // Configuration des topics MQTT

    static MqttEventHandlers &EventHandlers; // Gestionnaire des événements MQTT


public:
    void setMotorTaskHandles(TaskHandle_t motorRightHandle, TaskHandle_t motorLeftHandle);

    // Méthode pour démarrer la tâche MQTT
    static void run(void *pvParameters); 
    void execute(); // Méthode pour exécuter le traitement des messages MQTT
    void startTask(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority); // Démarre une nouvelle tâche


    // Méthodes liées aux capteurs
    static void runcapteursR(void *pvParameters); 
    void executecapteursR(); // Exécute le traitement des notifications des capteurs
    static void runcapteursL(void *pvParameters); 
    void executecapteursL(); // Exécute le traitement des notifications des capteurs
    void startTaskcapteursLeft(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority); // Démarre une tâche liée aux capteurs
    void startTaskcapteursRight(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority);

    // Méthodes liées au moteur
    static void runmotor(void *pvParameters); 
    void executemotor(); // Exécute les commandes moteur
    void startTaskmotor(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority); // Démarre une tâche pour le moteur

// Getters pour récupérer les handles des capteurs
    TaskHandle_t getTaskToCaptorRight();
    TaskHandle_t getTaskToCaptorLeft();

    typedef struct MQTT_USER
    {
        TaskHandle_t taskHandle;
        int32_t event_id;
        int topic_type;
        int topic_len;
        char topic[64];
        int data_len;
        char data[64];
    } MQTT_t;

    typedef struct
    {
        int client;
    } _config_message_t;

    // Configuration des informations de connexion MQTT
    void SetCredentials(const char *hostname, const char *username,
                        const char *client_id, const char *testament_topic,
                        const char *password);

   // Gestion des événements MQTT
    static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

    // Gestion des erreurs
    static void log_error_if_nonzero(const char *message, int error_code);

    MqttGeneral(TaskHandle_t taskHandle, QueueHandle_t commandQueuemotor, Config &config_topic, MqttEventHandlers &EventHandlers);
    MqttGeneral(TaskHandle_t taskHandle, QueueHandle_t commandQueuemotor, ConfigTmaze &configtm_topic, MqttEventHandlers &EventHandlers);
    MqttGeneral(QueueHandle_t commandQueuemotor,ConfigTmaze &configtm_topic, MqttEventHandlers &EventHandlers);

    ~MqttGeneral(void);

    static void mqtt_app_start();

    static esp_mqtt_client_config_t mqtt_cfg_client;
    static esp_mqtt_client_handle_t client;
    static _config_message_t client_message;
    static MQTT_t mqttBuffeeders;

    // static void TaskMqtt(void);

    static const char *TAG;

    // static StandardGpio::GpioOutput Led;
};

#endif // CPPMQTT_H