// GpioReaderpellet.cpp

#include "GpioReaderpellet.h"

/*GpioReaderpellet::GpioReaderpellet(gpio_num_t pin, TaskHandle_t taskTopellet) : taskTopellet(taskTopellet), pin(pin), taskHandle(nullptr)
{

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

void GpioReaderpellet::startMonitoring()
{
    xTaskCreate(&GpioReaderpellet::run, "monitorPinPelletTask", 2096, this, 1, &taskTopellet);
    ESP_LOGI("thread mqtt capteur ", "La tâche monitorPinPelletTask a été créée.num : %p", taskTopellet);
}

void GpioReaderpellet::run(void *arg)
{
    GpioReaderpellet *reader = static_cast<GpioReaderpellet *>(arg);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Vérifie l'état de la broche toutes les 10 ms

    while (true)
    {
        reader->execute();
    }
}

void GpioReaderpellet::execute()
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Vérifie l'état de la broche toutes les 5 ms
    int64_t currentTime = esp_timer_get_time();
    int level = gpio_get_level(pin);

    if (level == 1)
        { // État bas
            ESP_LOGI("reader pellet ", "Envoi d'une notification à ControlMotor");
            TaskNotifier::notify(taskTopellet, 3); // Notifier le
        }

}*/

//--------------------------------------------------------------------------
/*#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"

GpioReaderpellet::GpioReaderpellet(gpio_num_t pin, QueueHandle_t commandQueue) : commandQueue(commandQueue)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

void GpioReaderpellet::startMonitoring()
{
    xTaskCreate(monitorTask, "SensorMonitorTask", 2048, this, 5, nullptr);
}

void GpioReaderpellet::monitorTask(void *param)
{
            GpioReaderpellet* sensor = static_cast<GpioReaderpellet*>(param);

    while (true)
    {
        int sensorValue = gpio_get_level(GpioReaderpellet::pin); // Lire la valeur du capteur
        if (sensorValue == 1)
        {                    // Condition pour envoyer un signal, ajustez selon votre logique
            int command = 5; // Commande pour arrêter le moteur, par exemple
            xQueueSendToBack(sensor->commandQueue, &command, portMAX_DELAY);
            ESP_LOGW("sensor", "Commande capteur: %d", command);
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Fréquence de vérification du capteur
    }
}
*/

// Définition des broches du capteur
#include "esp_log.h"
#include "driver/gpio.h"

SensorTask::SensorTask(SemaphoreHandle_t stopSemaphore) : stopSemaphore(stopSemaphore) {}

void SensorTask::startMonitoring()
{
    xTaskCreate(monitorTask, "SensorMonitorTask", 2048, this, 5, nullptr);
}

void SensorTask::monitorTask(void *param)
{
    static_cast<SensorTask *>(param)->execute();
}

void SensorTask::execute()
{
    gpio_num_t pin = GPIO_NUM_23; // Exemple de broche à surveiller
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    BaseType_t xStatus;
    int lastDetectionTime = 0; // Initialiser le temps de la dernière détection à 0
    int command = 0;           // Commande pour arrêter le moteur, par exemple

    while (true)
    {
        bool stop = true;

        if (xSemaphoreTake(stopSemaphore, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGW("SensorTask", " Activation du capteur par le moteur");
            while (1)
            {

                int sensorValue = gpio_get_level(pin); // Lire la valeur du capteur
                if (sensorValue == 1)
                {                                          // Condition pour envoyer un signal, ajustez selon votre logique
                    int currentTime = xTaskGetTickCount(); // Obtenir le timestamp actuel
                    if (lastDetectionTime > 0)
                    {                                                   // S'assurer que ce n'est pas la première détection
                        int duration = currentTime - lastDetectionTime; // Calculer la durée entre les détections
                        ESP_LOGI("SensorTask", "Durée depuis la dernière détection : %lu ms", pdTICKS_TO_MS(duration));
                    }
                    lastDetectionTime = currentTime; // Mettre à jour le temps de la dernière détection
                    if (xSemaphoreGive(stopSemaphore) == pdPASS)
                    {
                        ESP_LOGW("SensorTask", "Commande envoyé au motor par le capteur");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        break;
                    };
                }
                vTaskDelay(pdMS_TO_TICKS(4));
            }
        }
    }
}
