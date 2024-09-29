#include "TaskNotifier.h" // Assurez-vous que TaskNotifier est bien défini
#include "esp_log.h"

static const char *TAG = "motor_task";

#include "ControlMotor.h"

#include "esp_log.h"
OutPutTTL OutPTTL(GPIO_NUM_14);

// OutPutTTL MotorTask::outTTL = OutPTTL;

MotorTask::MotorTask(SemaphoreHandle_t stopSemaphore,
                     QueueHandle_t commandQueuemotor,
                     gpio_num_t stepPin, gpio_num_t dirPin) : stopSemaphore(stopSemaphore),
                                                              commandQueuemotor(commandQueuemotor),
                                                              motor(stepPin, dirPin), changement_direction(0), nb_pas_faible(0), outTTL(OutPTTL) {}

TaskHandle_t MotorTask::getTaskHandle()
{
    return taskHandle;
}

void MotorTask::start()
{
    xTaskCreate(&MotorTask::run, "MotorTask", 2048, this, 5, &taskHandle);
}

void MotorTask::run(void *param)
{
    static_cast<MotorTask *>(param)->execute();
}

void MotorTask::execute()
{
    while (true)
    {
        int ulNotificationValue = TaskNotifier::waitForNotification(taskHandle);

        // ESP_LOGI(TAG, "mqtt value notification : %d", ulNotificationValue);
        // Vérifier la valeur de la notification pour déterminer l'action
        switch (ulNotificationValue)
        {
        case 1: // Code 1 pour aller à la position 1
            // ESP_LOGI(TAG, "Déplacement vers la position initiale");
            movement();
            break;
        case 2: // Code 2 pour retourner à la position 2
            ESP_LOGI(TAG, "Retour à la position");
            // moveBetweenPositions(45, 15, 1000);
            break;
        default:
            ESP_LOGI(TAG, "Notification inconnue: %d", ulNotificationValue);
            break;
        }
    }
}
void MotorTask::performStep(bool direction)
{
    motor.setDirection(direction);
    motor.stepMotor(); // Votre logique pour faire un pas avec le moteur
}

void MotorTask::movement()
{
    bool direction = true;
    int nb_pas = 0;
    int nb_changement_direction = 0;
    int nb_nb_pas_faible = 0;

    if (xSemaphoreGive(stopSemaphore) == pdPASS)
    {
        ESP_LOGW(TAG, "activation du capteur start ");

        while (1)
        {
            nb_pas++;
            // ESP_LOGI(TAG, "nouveau pas : %d", nb_pas);

            performStep(direction); // Effectue un pas

            if (xSemaphoreTake(stopSemaphore, 0) == pdTRUE)
            {
                ESP_LOGI("MotorTask", "Commande envoyé par le capteur");
                ESP_LOGI("MotorTask", "nb_pas: %d, changement de direction: %d, nb pas faible: %d", nb_pas, changement_direction, nb_pas_faible);
                if (nb_pas < 30)
                {

                    if (nb_nb_pas_faible >= 2)
                    {
                        int command = 6;
                        xQueueSend(commandQueuemotor, &command, portMAX_DELAY);
                        outTTL.sortiTTL.on();
                        vTaskDelay(pdMS_TO_TICKS(50));
                        outTTL.sortiTTL.off();

                        ESP_LOGW(TAG, "fin de sequance motor envoie au mqtt ");
                        nb_pas = 0;
                        // nb_changement_direction = 0;
                        //  stop = false; // N
                        break;
                    }
                    else
                    {
                        vTaskDelay(pdMS_TO_TICKS(1000));

                        xSemaphoreGive(stopSemaphore);
                        ESP_LOGW(TAG, "nb de pas faible ");
                        nb_pas_faible++;
                        nb_pas = 0;
                    }
                    nb_nb_pas_faible++;

                    // vTaskDelay(pdMS_TO_TICKS(1000));
                }
                else if (nb_pas > 400)
                {

                    if (nb_changement_direction == 1)
                    {
                        int command = 6;
                        xQueueSend(commandQueuemotor, &command, portMAX_DELAY);
                        outTTL.sortiTTL.on();
                        vTaskDelay(pdMS_TO_TICKS(50));
                        outTTL.sortiTTL.off();
                        ESP_LOGW(TAG, "fin de sequance motor envoie au mqtt ");
                        nb_pas = 0;
                        nb_changement_direction = 0;
                        // stop = false; // N
                        break;
                    }
                    else
                    {
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        xSemaphoreGive(stopSemaphore);

                        direction = false;
                        ESP_LOGW(TAG, "changement de direction ");
                        changement_direction++;
                        nb_pas = 0;
                    }
                    nb_changement_direction++;
                }
                else
                {
                    int command = 6;
                    xQueueSend(commandQueuemotor, &command, portMAX_DELAY);
                    outTTL.sortiTTL.on();
                    vTaskDelay(pdMS_TO_TICKS(50));
                    outTTL.sortiTTL.off();
                    ESP_LOGW(TAG, "fin de sequance motor envoie au mqtt ");
                    nb_pas = 0;
                    // stop = false; // N
                    break;
                }
            }
        }
    }
    // vTaskDelay(pdMS_TO_TICKS(100));
}