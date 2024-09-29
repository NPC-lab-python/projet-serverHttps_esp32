#include "TaskNotifier.h"
#include "esp_log.h"

static const char* TAG = "TaskNotifier";

void TaskNotifier::notify(TaskHandle_t taskToNotify, int ulValue) {
    ESP_LOGI(TAG, "Envoi d'une notification à la tâche %p avec la valeur %d", taskToNotify, ulValue);
    xTaskNotify(taskToNotify, ulValue, eSetValueWithOverwrite);
}


int TaskNotifier::waitForNotification(TaskHandle_t task, TickType_t ticksToWait) {
    //ESP_LOGI(TAG, "Tâche %p attendant une notification", task);
    int notificationReceived = ulTaskNotifyTake(pdTRUE, ticksToWait);
    if (notificationReceived) {
        ESP_LOGI(TAG, "Tâche %p a reçu une notification", task);
    } else {
        //ESP_LOGI(TAG, "Tâche %p n'a pas reçu de notification dans le temps imparti", task);
    }
    return notificationReceived;
}

