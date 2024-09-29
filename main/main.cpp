// main/main.cpp

#include "WifiCpp.h"
#include "CPPMqtt.h"
#include "ControlMotor.h"
#include "freertos/semphr.h"
#include "Configurations.h"
#include "MqttEventHandlers.h"
#include "ServoSCS15.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "main.h"
#include "C:/Users/galileo/ServerHttpsEsp32/include/https_server.h" // Inclure le header de la bibliothèque
#include "esp_tls.h"

#include "nvs_flash.h"

static const char *TAG = "APP_MAIN";

// Déclaration de l'instance globale de MqttGeneral
MqttGeneral *mymqtt = nullptr;

// Fonctions pour sauvegarder et charger les paramètres MQTT
void save_mqtt_credentials(const char* hostname, const char* username, const char* password, const char* client_id, const char* testament_topic) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_set_str(nvs_handle, "hostname", hostname);
        nvs_set_str(nvs_handle, "username", username);
        nvs_set_str(nvs_handle, "password", password);
        nvs_set_str(nvs_handle, "client_id", client_id);
        nvs_set_str(nvs_handle, "testament_topic", testament_topic);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
                ESP_LOGI(TAG, "Paramètres MQTT sauvegardés dans la NVS");

    } else {
        ESP_LOGE(TAG, "Erreur lors de l'ouverture de la NVS pour l'écriture (%s)", esp_err_to_name(err));
    }
}

void load_mqtt_credentials(char* hostname, size_t hostname_size,
                           char* username, size_t username_size,
                           char* password, size_t password_size,
                           char* client_id, size_t client_id_size,
                           char* testament_topic, size_t testament_topic_size) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        esp_err_t res;

        // Charger le hostname
        res = nvs_get_str(nvs_handle, "hostname", hostname, &hostname_size);
        if (res != ESP_OK) {
            ESP_LOGW(TAG, "Clé 'hostname' non trouvée (%s)", esp_err_to_name(res));
        } else {
            // Vérifier si le hostname contient un schéma
            if (strncmp(hostname, "mqtt://", 7) != 0 && strncmp(hostname, "mqtts://", 8) != 0 &&
                strncmp(hostname, "ws://", 5) != 0 && strncmp(hostname, "wss://", 6) != 0) {
                // Ajouter "mqtt://" par défaut
                char corrected_hostname[256]; // Taille maximale pour le hostname corrigé
                snprintf(corrected_hostname, sizeof(corrected_hostname), "mqtt://%s", hostname);
                // Copier le hostname corrigé dans la variable hostname
                strncpy(hostname, corrected_hostname, hostname_size);
                hostname[hostname_size - 1] = '\0'; // S'assurer que la chaîne est terminée par un caractère nul
                ESP_LOGI(TAG, "Schéma ajouté au hostname : %s", hostname);
            }
        }

        // Charger les autres paramètres
        res = nvs_get_str(nvs_handle, "username", username, &username_size);
        if (res != ESP_OK) ESP_LOGW(TAG, "Clé 'username' non trouvée (%s)", esp_err_to_name(res));

        res = nvs_get_str(nvs_handle, "password", password, &password_size);
        if (res != ESP_OK) ESP_LOGW(TAG, "Clé 'password' non trouvée (%s)", esp_err_to_name(res));

        res = nvs_get_str(nvs_handle, "client_id", client_id, &client_id_size);
        if (res != ESP_OK) ESP_LOGW(TAG, "Clé 'client_id' non trouvée (%s)", esp_err_to_name(res));

        res = nvs_get_str(nvs_handle, "testament_topic", testament_topic, &testament_topic_size);
        if (res != ESP_OK) ESP_LOGW(TAG, "Clé 'testament_topic' non trouvée (%s)", esp_err_to_name(res));

        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "Paramètres MQTT chargés depuis la NVS");
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Namespace 'mqtt' non trouvé, utilisation des valeurs par défaut");
        // Les variables restent inchangées, vous pouvez utiliser les valeurs par défaut
    } else {
        ESP_LOGE(TAG, "Erreur lors de l'ouverture de la NVS pour la lecture (%s)", esp_err_to_name(err));
    }
}


// Fonction de callback pour mettre à jour les paramètres MQTT
void mqtt_update_callback(const std::map<std::string, std::string>& params) {
    std::string hostname = params.at("hostname");
    std::string username = params.at("username");
    std::string password = params.at("password");
    std::string client_id = params.at("client_id");
    std::string testament_topic = params.at("testament_topic");

    if (mymqtt != nullptr) {
        mymqtt->SetCredentials(
            hostname.c_str(),
            username.c_str(),
            client_id.c_str(),
            testament_topic.c_str(),
            password.c_str()
        );

        // Enregistrer les paramètres dans la NVS
        save_mqtt_credentials(
            hostname.c_str(),
            username.c_str(),
            password.c_str(),
            client_id.c_str(),
            testament_topic.c_str()
        );

        // Redémarrer le client MQTT avec les nouveaux paramètres
        esp_mqtt_client_stop(MqttGeneral::client);
        esp_mqtt_client_destroy(MqttGeneral::client);

        // Réinitialiser le client MQTT avec les nouveaux paramètres
        MqttGeneral::client = esp_mqtt_client_init(&MqttGeneral::mqtt_cfg_client);
        mymqtt->mqtt_app_start();
    }
}

extern "C" void app_main(void) {
    // Initialisation de la NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    int valconfig = 2;

    Config config;
    ConfigTmaze configTm;

    std::string nameHost;
    std::string usernamemqtt;
    std::string clientIDmqtt;
    std::string testament;
    std::string passmqtt;
    std::string iphostmqtt;

    switch (valconfig)
    {
    case 1:

        config = Config("3c/01/F01/$connected",
                        "3c/01/F01/feeder_1/$meta",
                        "3c/01/F01/feeder_1/activate/$meta",
                        "3c/01/F01/feeder_1/pellet_delivered/$meta",
                        "3c/01/F01/feeder_1/pellet_delivered",
                        "3c/01/F01/feeder_1/nose_poke",
                        "3c/01/F01/feeder_1/beam_pellet",
                        "3c/01/F01/feeder_1/activate/set", "2");
        nameHost = "feeder-sasha01";
        usernamemqtt = "3cfeeders1";
        clientIDmqtt = "3cfeeders_01";
        testament = "3c/01/F01/$connected";
        passmqtt = "3cfeeders1";
        iphostmqtt = "10.0.0.99";

        break;
    case 2:

        configTm = ConfigTmaze("souris_city/01/SC01T01/$connected",
                               "{\"type\": \"servo\", \"location\": \"T_MAZE\",\"properties\":[{\"name\":\"servo_move\", \"datatype\": \"str\", \"settable\": true, \"retain\": false}]}",
                               "{\"type\": \"servo\", \"location\": \"T_MAZE\",\"properties\":[{\"name\":\"servo_move\", \"datatype\": \"str\", \"settable\": true, \"retain\": false}]}",
                               "{\"type\": \"beams\", \"location\": \"T_MAZE\",\"properties\":[{\"name\":\"beam_broken\", \"datatype\": \"str\", \"settable\": false, \"retain\": false}]}",
                               "{\"type\": \"beams\", \"location\": \"T_MAZE\",\"properties\":[{\"name\":\"beam_broken\", \"datatype\": \"str\", \"settable\": false, \"retain\": false}]}",
                               "souris_city/01/SC01T01/servo_left/$meta",
                               "souris_city/01/SC01T01/servo_right/$meta",
                               "souris_city/01/SC01T01/beam_left/$meta",
                               "souris_city/01/SC01T01/beam_right/$meta",
                               "souris_city/01/SC01T01/servo_left/beam_broken",
                               "souris_city/01/SC01T01/servo_right/beam_broken",
                               "souris_city/01/SC01T01/beam_left/beam_broken",
                               "souris_city/01/SC01T01/beam_right/beam_broken",
                               "souris_city/01/SC01T01/activate/set",
                               "1");
        ESP_LOGI(TAG, "ConfigTmaze initialized");

        nameHost = "souris_city";
        usernamemqtt = "souris_city";
        clientIDmqtt = "souris_city";
        testament = "souris_city/01/SC01T01/$connected";
        passmqtt = "souris_city";
        // iphostmqtt = "192.168.137.1";
        iphostmqtt = "10.0.0.99";

        break;
    default:
        break;
    }

    MqttEventHandlers MqttHandlers;

    // ---- Wi-Fi
    WIFI::Wifi myWifi;
    myWifi.SetCredentials("maison", "galileo@9493!", "server https"); // Remplacez par vos informations Wi-Fi
    myWifi.TaskWifi();


    // Variable pour contrôler l'utilisation de HTTPS
    bool use_https = false; // Changez à `false` pour utiliser HTTP
  // Initialisation du serveur HTTP/HTTPS
    HttpsServer httpsServer(use_https);
    httpsServer.setMqttUpdateCallback(mqtt_update_callback);
    httpsServer.start();

    // Charger les paramètres MQTT depuis la NVS
    char hostname[64] = "";
    char username[32] = "";
    char password[32] = "";
    char client_id[32] = "";
    char testament_topic[64] = "";

    // load_mqtt_credentials(hostname, sizeof(hostname), username, sizeof(username), password, sizeof(password), client_id, sizeof(client_id), testament_topic, sizeof(testament_topic));

    // Si les paramètres sont vides, utiliser les valeurs par défaut ou celles de la configuration
    if (strlen(hostname) == 0) {
        strcpy(hostname, iphostmqtt.c_str());
        strcpy(username, usernamemqtt.c_str());
        strcpy(password, passmqtt.c_str());
        strcpy(client_id, clientIDmqtt.c_str());
        strcpy(testament_topic, testament.c_str());
    }

    // ---- MQTT

    QueueHandle_t commandQueuemotor = xQueueCreate(10, sizeof(int));

    // ----------------------------------------------------------------------------------------------------------------
    // Créer une instance de ServoSCS15
    ServoSCS15 servo(UART_NUM_1, GPIO_NUM_17, GPIO_NUM_16, 2, 1000000); // Ajustez les numéros de GPIO si nécessaire
    servo.init();

    uint8_t motor1_id = 1;
    uint8_t motor2_id = 2;

    servo.start_motor_move_only_thread(motor1_id, 0, 1500, "motorRight move");
    servo.start_motor_move_only_thread(motor2_id, 1000, 1400, "motorLeft move");

    TaskHandle_t moteur1TaskHandle = xTaskGetHandle("motorRight move");
    TaskHandle_t moteur2TaskHandle = xTaskGetHandle("motorLeft move");

    // ----- MQTT
    // Initialisation de l'objet MqttGeneral selon la configuration choisie
    mymqtt = new MqttGeneral(commandQueuemotor, configTm, MqttHandlers);

    if (mymqtt != nullptr)
    {
        mymqtt->setMotorTaskHandles(moteur1TaskHandle, moteur2TaskHandle);
        mymqtt->SetCredentials(hostname, username, client_id, testament_topic, password);
        mymqtt->startTask("MqttThread", 8192, 4);
        mymqtt->startTaskcapteursRight("mqttThreadCapteurRight", 2048, 5);
        mymqtt->startTaskcapteursLeft("mqttThreadCapteurLeft", 2048, 5);
        mymqtt->startTaskmotor("mqttThreadmotor", 2048, 5);
        mymqtt->mqtt_app_start();
    }

    TaskHandle_t readerLeftHandle = mymqtt->getTaskToCaptorLeft();

    GpioReader readerLeft(GPIO_NUM_35, 2, 50000, readerLeftHandle);
    readerLeft.startMonitoringBeam();

    // Boucle infinie
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
