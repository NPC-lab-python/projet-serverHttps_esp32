#include "MotorDriver.h"
#include "esp_log.h"

MotorDriver::MotorDriver(gpio_num_t stepPin, gpio_num_t dirPin) : stepPin(stepPin), dirPin(dirPin) {
    initializePins();
}

esp_err_t MotorDriver::_init(const gpio_num_t pin)
    {
        esp_err_t status{ESP_OK};

        gpio_config_t cfg;
        cfg.pin_bit_mask = 1ULL << pin;
        cfg.mode = GPIO_MODE_OUTPUT;
        cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        cfg.intr_type = GPIO_INTR_DISABLE;

        status |= gpio_config(&cfg);

        return status;
    }

void MotorDriver::initializePins() {
    _init(stepPin);
    _init(dirPin);
    setDirection(1);
}

void MotorDriver::setDirection(bool direction) {
    gpio_set_level(dirPin, direction ? 1 : 0);
}

void MotorDriver::stepMotor() {
    gpio_set_level(stepPin, 1);
    vTaskDelay(pdMS_TO_TICKS(2)); // Utilisez ets_delay_us pour un délai court et précis
    gpio_set_level(stepPin, 0);
    vTaskDelay(pdMS_TO_TICKS(2));
    // ESP_LOGI("driver motor ", "un pas");

}
