// Servo.cpp
#include "Servo.h"
#include "esp_log.h"

static const char *TAG = "Servo_bottle_left";

Servo::Servo(int gpio, int minPulseWidthUs, int maxPulseWidthUs, int minDegree, int maxDegree)
    : _gpio(gpio), _minPulseWidthUs(minPulseWidthUs), _maxPulseWidthUs(maxPulseWidthUs),
      _minDegree(minDegree), _maxDegree(maxDegree), _angle(0), _step(2)
{

    initMcpwm();
}
TaskHandle_t Servo::getTaskHandle()
{
    return taskHandle;
}

void Servo::moveBetweenPositions(int position1, int position2, int delayMs)
{
    initposition(15);
    vTaskDelay(pdMS_TO_TICKS(5000));

    for (int i = 0; i < 2; ++i)
    { // Répéter deux fois pour aller et venir
        _angle = (i % 2 == 0) ? position1 : position2;
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(_comparator, angleToCompare(_angle)));
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}

void Servo::initposition(int position)
{
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(_comparator, angleToCompare(_angle)));
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void Servo::rotate()
{
    while (true)
    {
        ESP_LOGI(TAG, "Angle of rotation: %d", _angle);
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(_comparator, angleToCompare(_angle)));
        vTaskDelay(pdMS_TO_TICKS(500));
        if ((_angle + _step) > _maxDegree || (_angle + _step) < _minDegree)
        {
            _step *= -1;
        }
        _angle += _step;
    }
}

uint32_t Servo::angleToCompare(int angle)
{
    int a = (angle - _minDegree) * (_maxPulseWidthUs - _minPulseWidthUs) / (_maxDegree - _minDegree) + _minPulseWidthUs;
    ESP_LOGI(TAG, "valeur d'impulsion : %d", a);

    return a;
}

void Servo::initMcpwm()
{
    ESP_LOGI(TAG, "Create timer and operator");
    mcpwm_timer_config_t timer_config;
    timer_config.group_id = 0;
    timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    timer_config.resolution_hz = _SERVO_TIMEBASE_RESOLUTION_HZ;
    timer_config.period_ticks = _SERVO_TIMEBASE_PERIOD;
    timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;

    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &_timer));

    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config;
    operator_config.group_id = 0; // operator must be in the same group to the timer

    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &_oper));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(_oper, _timer));

    ESP_LOGI(TAG, "Create comparator and generator from the operator");
    mcpwm_comparator_config_t comparator_config;
    comparator_config.flags.update_cmp_on_tez = true;

    ESP_ERROR_CHECK(mcpwm_new_comparator(_oper, &comparator_config, &_comparator));

    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config;
    generator_config.gen_gpio_num = _gpio;

    ESP_ERROR_CHECK(mcpwm_new_generator(_oper, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(_comparator, angleToCompare(0)));

    ESP_LOGI(TAG, "Set generator action on timer and compare event");
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
                                                              MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, _comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(_timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP));

    // startTask("ServoThread", 4096, 3);
}

void Servo::startTask(const char *taskName, uint32_t stackSize, UBaseType_t taskPriority)
{
    xTaskCreate(&Servo::run, taskName, stackSize, this, taskPriority, &taskHandle);
}

void Servo::run(void *param)
{
    auto *servo = static_cast<Servo *>(param);
    // Ici, implémentez la logique de contrôle du servo, par exemple :
    servo->execute();
}

void Servo::execute()
{

    while (true)
    {
        ESP_LOGI(TAG, "Attendre la notification");
        int ulNotificationValue = TaskNotifier::waitForNotification(taskHandle);

        if (ulNotificationValue > 0) { // Une notification a été reçue
            // Vérifier la valeur de la notification pour déterminer l'action
            switch (ulNotificationValue) {
                case 1: // Code 1 pour aller à la position 1
                    ESP_LOGI(TAG, "Déplacement vers la position initiale");
                    moveBetweenPositions(15, 45, 1000);
                    break;
                case 2: // Code 2 pour retourner à la position 2
                    ESP_LOGI(TAG, "Retour à la position");
                    moveBetweenPositions(45, 15, 1000);
                    break;
                default:
                    ESP_LOGI(TAG, "Notification inconnue: %d", ulNotificationValue);
                    break;
            }
        }
    }
}