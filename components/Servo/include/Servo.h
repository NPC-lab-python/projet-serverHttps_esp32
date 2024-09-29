// Servo.h
#ifndef SERVO_H
#define SERVO_H

#include "driver/mcpwm_prelude.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TaskNotifier.h"

class Servo {
public:
    Servo(int gpio, int minPulseWidthUs, int maxPulseWidthUs, int minDegree, int maxDegree);
    void rotate();
    void moveBetweenPositions(int position1, int position2, int delayMs);
    void initposition(int position);
    TaskHandle_t getTaskHandle();

    static void run(void *param);
    void execute();
    void startTask(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority);


private:
    TaskHandle_t taskHandle;

    int _SERVO_TIMEBASE_RESOLUTION_HZ = 1000000;
    int _SERVO_TIMEBASE_PERIOD = 20000;
    int _gpio;
    int _minPulseWidthUs;
    int _maxPulseWidthUs;
    int _minDegree;
    int _maxDegree;
    int _angle;
    int _step;
    mcpwm_timer_handle_t _timer;
    mcpwm_oper_handle_t _oper;
    mcpwm_cmpr_handle_t _comparator;

    uint32_t angleToCompare(int angle);
    void initMcpwm();
};

#endif // SERVO_H
