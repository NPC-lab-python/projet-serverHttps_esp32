#ifndef TASK_NOTIFIER_H
#define TASK_NOTIFIER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class TaskNotifier {
public:
    static void notify(TaskHandle_t taskToNotify, int ulValue);
    static int waitForNotification(TaskHandle_t task, TickType_t ticksToWait = portMAX_DELAY);
};

#endif // TASK_NOTIFIER_H
