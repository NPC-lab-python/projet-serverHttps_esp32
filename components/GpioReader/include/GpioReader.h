
#include "driver/gpio.h"
#include "esp_timer.h"
#include "TaskNotifier.h"

class GpioReader
{
public:
    GpioReader(gpio_num_t pin, int config, int64_t debounceTimeInUs = 50000, TaskHandle_t taskToCaptor = nullptr);
    void startMonitoring();
    void startMonitoringBeam();

    static void monitorPinTask(void *arg);
    static void monitorBeamTask(void *arg);

    static int lastState;

private:
    TaskHandle_t taskToCaptor; // Handle de la tâche à notifier
    // TaskHandle_t taskTopellet; // Handle de la tâche à notifier

    gpio_num_t pin;
    int64_t lastLowToHighTransition;
    int64_t lastDebounceTime;
    int64_t debounceTimeInUs;
    TaskHandle_t taskHandle;
    int valuenotif;
    int config;
    void handleTransition();
    void handleTransitionbeam();

};
