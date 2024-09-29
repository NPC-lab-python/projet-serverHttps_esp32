// GpioReaderpellet.h
#ifndef GPIOREADERPELLET_H
#define GPIOREADERPELLET_H
#pragma once

#include "driver/gpio.h"
#include "esp_timer.h"
#include "TaskNotifier.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


/*class GpioReaderpellet {
public:
    GpioReaderpellet(gpio_num_t pin, TaskHandle_t taskTopellet = nullptr);
    void startMonitoring();
    static void run(void* arg);

private:
    TaskHandle_t taskTopellet; // Handle de la tâche à notifier
    gpio_num_t pin;
    TaskHandle_t taskHandle;
    void execute();
};
class GpioReaderpellet {
public:
    GpioReaderpellet(gpio_num_t pin, QueueHandle_t commandQueue);
    void startMonitoring();

private:
    static gpio_num_t pin;
    QueueHandle_t commandQueue; // Queue pour envoyer des commandes à ControlMotor
    static void monitorTask(void* param);
};*/



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"


class SensorTask {
public:
    SensorTask(SemaphoreHandle_t stopSemaphore);
    void startMonitoring();

private:
    static void monitorTask(void *param);
    void execute();

    SemaphoreHandle_t stopSemaphore;
};

#endif //GPIOREADERPELLET_H