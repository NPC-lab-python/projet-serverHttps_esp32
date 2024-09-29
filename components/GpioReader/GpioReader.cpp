// GpioReader.cpp

#include "GpioReader.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

GpioReader::GpioReader(gpio_num_t pin, int config, int64_t debounceTimeInUs, TaskHandle_t taskToCaptor) : taskToCaptor(taskToCaptor),
                                                                                                          pin(pin),
                                                                                                          lastLowToHighTransition(0),
                                                                                                          lastDebounceTime(0),
                                                                                                          debounceTimeInUs(debounceTimeInUs),
                                                                                                          taskHandle(nullptr),
                                                                                                          config(config)
{

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

int GpioReader::lastState = -1;

void GpioReader::startMonitoring()
{
    xTaskCreate(&GpioReader::monitorPinTask, "monitorPinTask", 2096, this, 5, &taskHandle);
}
void GpioReader::startMonitoringBeam()
{
    xTaskCreate(&GpioReader::monitorBeamTask, "monitorPinTask", 2096, this, 5, &taskHandle);
}

void GpioReader::monitorPinTask(void *arg)
{
    GpioReader *reader = static_cast<GpioReader *>(arg);
    vTaskDelay(pdMS_TO_TICKS(100)); 

    while (true)
    {
        reader->handleTransition();
    }
}
void GpioReader::monitorBeamTask(void *arg)
{
    GpioReader *reader = static_cast<GpioReader *>(arg);
    vTaskDelay(pdMS_TO_TICKS(100)); 

    while (true)
    {
        reader->handleTransitionbeam();
    }
}

void GpioReader::handleTransition()
{
    int64_t currentTime = esp_timer_get_time();
    int level = gpio_get_level(pin);
    // ESP_LOGI("GpioReader", "config: %d ", config);

    vTaskDelay(pdMS_TO_TICKS(100)); // Vérifie l'état de la broche toutes les 100 ms
    // Vérifiez si l'état actuel diffère de l'état précédent
    if (level != lastState && (currentTime - lastDebounceTime) > debounceTimeInUs)
    {
        ESP_LOGI("GpioReader nosepoke", " état : %d , laststate : %d", level, lastState);

        // Mettre à jour le temps du dernier rebond pour filtrer les prochains rebonds
        lastDebounceTime = currentTime;
        int valuenotif = 4;
        if (level == 0)
        { // État bas
            int64_t duration = currentTime - lastLowToHighTransition;
            ESP_LOGI("GpioReader nosepoke", "Temps depuis dernier état bas: %lld microseconds", duration);
            TaskNotifier::notify(taskToCaptor, 4); // Notifier le mqtt
            lastLowToHighTransition = currentTime; // Réinitialiser le timer
        }
        lastDebounceTime = currentTime;

        // Mettre à jour l'état précédent pour la prochaine comparaison
        lastState = level;
    }
}

void GpioReader::handleTransitionbeam()
{
    int64_t currentTime = esp_timer_get_time();
    int level = gpio_get_level(pin);

    vTaskDelay(pdMS_TO_TICKS(100)); // Vérifie l'état de la broche toutes les 100 ms
    // Vérifiez si l'état actuel diffère de l'état précédent
    if (level != lastState && (currentTime - lastDebounceTime) > debounceTimeInUs)
    {
        ESP_LOGI("GpioReader beam", " état : %d , laststate : %d", level, lastState);

        // Mettre à jour le temps du dernier rebond pour filtrer les prochains rebonds
        lastDebounceTime = currentTime;
        int valuenotif = 4;
        if (level == 0)
        { // État bas
            int64_t duration = currentTime - lastLowToHighTransition;
            ESP_LOGI("GpioReader beam", "Temps depuis dernier état bas: %lld microseconds", duration);
            TaskNotifier::notify(taskToCaptor, valuenotif); // Notifier le mqtt
            lastLowToHighTransition = currentTime; // Réinitialiser le timer
        }
        lastDebounceTime = currentTime;

        // Mettre à jour l'état précédent pour la prochaine comparaison
        lastState = level;
    }
}