#ifndef WIFICPP_H
#define WIFICPP_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_wifi.h"

namespace WIFI
{

    class Wifi
    {
    public:
        enum class State
        {
            NOT_INITIALIZED,
            INITIALIZED,
            READY_TO_CONNECT,
            CONNECTING,
            WAITING_FOR_IP,
            CONNECTED,
            DISCONNECTED,
            ERROR
        };

        Wifi();
        ~Wifi();
        void stopTask();
        void startTask();
        void run();
        static void Taskrun(void *pvParameters);
        void TaskWifi();
        void SetCredentials(const char *ssid, const char *password, const char *hostname);
        esp_err_t Init();
        esp_err_t Begin();

        static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
        static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
        const char *getStateString(State state);
        constexpr static const State &GetState(void) { return _state; }

    private:
        char _mac_addr_cstr[18]{}; // MAC address in string format
        static SemaphoreHandle_t _mutx;
        SemaphoreHandle_t _taskControlMutex;
        static State _state;
        static wifi_init_config_t _wifi_init_cfg;
        static wifi_config_t _wifi_cfg;
        bool _state_wifi;
        bool _continueTask;
        const char* hostname;

        esp_err_t _init();
        esp_err_t _get_mac();
    };

} // namespace WIFI

#endif // WIFICPP_H
