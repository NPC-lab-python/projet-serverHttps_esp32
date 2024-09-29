#ifndef CONTROLMOTOR_H
#define CONTROLMOTOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MotorDriver.h" // Assurez-vous d'inclure votre implémentation de MotorDriver
#include "freertos/queue.h"

/*class ControlMotor {
public:
    ControlMotor(gpio_num_t stepPin, gpio_num_t dirPin);
    ~ControlMotor();
    static void run(void *pvParameters);
    void execute();
    void startTask(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority);
    TaskHandle_t getTaskHandle();
    TaskHandle_t getTaskHandlepellet();

    void performStep(); // Effectue un pas du moteur
    void movement(void);

private:
    TaskHandle_t taskTopellet; // Handle de la tâche à notifier
    TaskHandle_t taskHandle;
    const char* taskName;
    MotorDriver motor; // Instance de votre driver de moteur
};

class ControlMotor
{
public:
    ControlMotor(gpio_num_t stepPin, gpio_num_t dirPin, QueueHandle_t commandQueue);
    ~ControlMotor();

    void startMotorTask();
    void startSensorTask();

private:
    static void motorTask(void *param);
    static void sensorTask(void *param);

    void executeMotorSteps();
    void performStep();

    TaskHandle_t motorTaskHandle = nullptr;
    TaskHandle_t sensorTaskHandle = nullptr;
    static QueueHandle_t commandQueue; // Queue partagée pour les commandes entre les tâches
    MotorDriver motor;          // Adaptez cela à la classe de votre moteur
};
*/
#include "OutPutTTL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

class MotorTask
{
public:
    MotorTask(SemaphoreHandle_t stopSemaphore, QueueHandle_t commandQueuemotor, gpio_num_t stepPin, gpio_num_t dirPin);
    void start();
    TaskHandle_t getTaskHandle();
    void movement();

private:

    static void run(void *param);
    void execute();
    void performStep(bool direction);

    SemaphoreHandle_t stopSemaphore;
    QueueHandle_t commandQueuemotor;

    MotorDriver motor; // Adaptez cela à la classe de votre moteur
    TaskHandle_t taskHandle;
    int8_t changement_direction;
    int8_t nb_pas_faible;
    OutPutTTL outTTL;

};

#endif // CONTROLMOTOR_H
