#include "WifiCpp.h"
#include <cstring>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/semphr.h"

namespace WIFI
{

    SemaphoreHandle_t Wifi::_mutx = xSemaphoreCreateMutex();
    wifi_config_t Wifi::_wifi_cfg = {};                                   // Définition de _wifi_cfg
    WIFI::Wifi::State Wifi::_state = WIFI::Wifi::State::NOT_INITIALIZED;  // Définition et initialisation de _state
    wifi_init_config_t Wifi::_wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT(); // Configuration WiFi par défaut

    Wifi::Wifi()
    {
        _taskControlMutex = xSemaphoreCreateMutex();
        _state_wifi = false;
        _continueTask = true;
    }

    Wifi::~Wifi()
    {
        if (_mutx)
        {
            vSemaphoreDelete(_mutx);
        }
        if (_taskControlMutex)
        {
            vSemaphoreDelete(_taskControlMutex);
        }
    }
    const char *Wifi::getStateString(State state)
    {
        switch (state)
        {
        case State::NOT_INITIALIZED:
            return "NOT_INITIALIZED";
        case State::INITIALIZED:
            return "INITIALIZED";
        case State::READY_TO_CONNECT:
            return "READY_TO_CONNECT";
        case State::CONNECTING:
            return "CONNECTING";
        case State::WAITING_FOR_IP:
            return "WAITING_FOR_IP";
        case State::CONNECTED:
            return "CONNECTED";
        case State::DISCONNECTED:
            return "DISCONNECTED";
        case State::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    void Wifi::stopTask()
    {
        xSemaphoreTake(_taskControlMutex, portMAX_DELAY);
        _continueTask = false;
        xSemaphoreGive(_taskControlMutex);
    }

    void Wifi::startTask()
    {
        xSemaphoreTake(_taskControlMutex, portMAX_DELAY);
        _continueTask = true;
        xSemaphoreGive(_taskControlMutex);
    }

    void Wifi::run()
    {

        switch (_state)
        {
        case State::READY_TO_CONNECT:
            Begin(); // Tentative de connexion
            break;

        case State::CONNECTING:
            // Ici, vous pouvez ajouter une logique spécifique, comme un timeout pour la connexion
            break;

        case State::WAITING_FOR_IP:
            // Ici, vous pouvez gérer des actions pendant l'attente de l'IP
            break;

        case State::CONNECTED:
            // Actions à effectuer une fois connecté
            _continueTask = false;
            break;

        case State::DISCONNECTED:
            _state = State::READY_TO_CONNECT; // Définir l'état pour retenter une connexion
            break;

        case State::NOT_INITIALIZED:
        case State::INITIALIZED:
        case State::ERROR:
            // Traitement des états d'erreur ou non initialisés
            break;
        }
    }

    void Wifi::Taskrun(void *pvParameters)
    {
        auto wifiInstance = static_cast<Wifi *>(pvParameters);
        while (true)
        {
            /*State wifiState = GetState();
            const char *val =wifiInstance->getStateString(wifiState);
            ESP_LOGI("Wifi",  " %s: ",val);*/

            wifiInstance->run();

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    void Wifi::TaskWifi()
    {
        // Initialiser le système de fichiers NVS, nécessaire pour la gestion des paramètres WiFi
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
        esp_event_loop_create_default();

        // Initialiser le WiFi
        Init();
        ESP_LOGI("Wifi", "Initialisation terminé");

        // Créer la tâche WiFi
        startTask();
        xTaskCreate(&Wifi::Taskrun, "wifi_task", 4096, this, 5, NULL);
    }

    void Wifi::SetCredentials(const char *ssid, const char *password, const char *hostname)
    {
        strncpy((char *)_wifi_cfg.sta.ssid, ssid, sizeof(_wifi_cfg.sta.ssid));
        strncpy((char *)_wifi_cfg.sta.password, password, sizeof(_wifi_cfg.sta.password));
        this->hostname = hostname;
    }

    esp_err_t Wifi::Init()
    {

        esp_err_t status = ESP_OK;

        if (_state == State::NOT_INITIALIZED)
        {
            status |= esp_netif_init();
            if (ESP_OK == status)
            { // Création de l'interface par défaut si elle n'existe pas
                const esp_netif_t *const p_netif = esp_netif_create_default_wifi_sta();

                if (!p_netif)
                {
                    status = ESP_FAIL;
                    //_state = State::ERROR;
                }
                ESP_LOGI("Wifi", "Création de l'interface par défaut si elle n'existe pas");
            }

            if (ESP_OK == status)
            {
                status = esp_wifi_init(&_wifi_init_cfg);
                ESP_LOGI("Wifi", "initialisation de la config");
            }
            if (ESP_OK == status)
            {
                // Enregistrement des gestionnaires d'événements WiFi et IP
                status = esp_event_handler_instance_register(WIFI_EVENT,
                                                             ESP_EVENT_ANY_ID,
                                                             &wifi_event_handler,
                                                             nullptr,
                                                             nullptr);
                ESP_LOGI("Wifi", "enregistrement du gestionnaire wifi");
            }
            if (ESP_OK == status)
            {
                ESP_LOGI("Wifi", "start enregistrement du gestionnaire ip");

                status = esp_event_handler_instance_register(IP_EVENT,
                                                             ESP_EVENT_ANY_ID,
                                                             &ip_event_handler,
                                                             nullptr,
                                                             nullptr);
                ESP_LOGI("Wifi", "finish enregistrement du gestionnaire ip");
            }

            if (ESP_OK == status)
            {
                status = esp_wifi_set_mode(WIFI_MODE_STA);
                ESP_LOGI("Wifi", "set mode sta");
            }

            if (ESP_OK == status)
            {
                _wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
                _wifi_cfg.sta.pmf_cfg.capable = true;
                _wifi_cfg.sta.pmf_cfg.required = false;
                esp_netif_t *wifi_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
                esp_err_t err = esp_netif_set_hostname(wifi_netif, this->hostname);
                status = esp_wifi_set_config(WIFI_IF_STA, &_wifi_cfg);
                ESP_LOGI("Wifi", "set config ");
            }

            if (ESP_OK == status)
            {
                status = esp_wifi_start(); // start Wifi
                ESP_LOGI("Wifi", "start wifi ");
            }

            if (ESP_OK == status)
            {
                _state = State::INITIALIZED;
            }
        }
        else if (State::ERROR == _state)
        {
            ESP_LOGI("Wifi", "Erreur ");

            _state = State::NOT_INITIALIZED;
        }
        return status;
    }

    esp_err_t Wifi::Begin()
    {

        esp_err_t status = ESP_OK;

        switch (_state)
        {
        case State::READY_TO_CONNECT:
            ESP_LOGI("Wifi", "Prêt à se connecter");

        case State::DISCONNECTED:
            status = esp_wifi_connect();
            if (status == ESP_OK)
            {
                _state = State::CONNECTING;
                ESP_LOGI("Wifi", "methode begin Tentative de connexion au WiFi");
            }
            else
            {
                ESP_LOGE("Wifi", "Échec de la connexion, erreur: %s", esp_err_to_name(status));
            }
            break;

        case State::CONNECTING:
            ESP_LOGI("Wifi", "Connexion en cours");

        case State::WAITING_FOR_IP:
            ESP_LOGI("Wifi", "En attente d'une adresse IP");

        case State::CONNECTED:
            // Aucune action nécessaire si déjà en connexion ou connecté
            ESP_LOGI("Wifi", "Connecté au WiFi");
            break;

        case State::NOT_INITIALIZED:
        case State::INITIALIZED:
            ESP_LOGI("Wifi", "Wifi initialisé");

        case State::ERROR:
            status = ESP_FAIL;
            ESP_LOGE("Wifi", "WiFi non prêt pour la connexion, état: %s", getStateString(_state));
            break;
        }

        return status;
    }

    void Wifi::wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        if (WIFI_EVENT == event_base)
        {
            const wifi_event_t event_type = static_cast<wifi_event_t>(event_id);

            switch (event_type)
            {
            case WIFI_EVENT_STA_START:
                ESP_LOGI("Wifi", "wifi handler STA Start");
                _state = State::READY_TO_CONNECT;
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI("Wifi", "wifi handler STA Connected");
                _state = State::WAITING_FOR_IP;
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI("Wifi", "wifi handler STA Disconnected");
                _state = State::DISCONNECTED;
                break;
            default:
                // ESP_LOGW("Wifi", "Unhandled WiFi Event: %d", event_id);
                break;
            }
        }
    }

    void Wifi::ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {

        if (IP_EVENT == event_base)
        {
            const ip_event_t event_type = static_cast<ip_event_t>(event_id);

            switch (event_type)
            {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI("Wifi", "IP Obtenu");
                _state = State::CONNECTED;
                break;

            case IP_EVENT_STA_LOST_IP:
                ESP_LOGI("Wifi", "IP Perdu");
                _state = State::DISCONNECTED;
                break;

            default:
                // ESP_LOGW("Wifi", "Événement IP non géré: %d", event_id);
                break;
            }
        }
    }

} // namespace WIFI

/*
backtrace
C:\Espressif_502\Espressif\tools\xtensa-esp32-elf\esp-2022r1-11.2.0\xtensa-esp32-elf\bin\xtensa-esp32-elf-addr2line.exe -pfiaC -e build/bottle_system_esp32_v1.elf 0x400d685a
*/
