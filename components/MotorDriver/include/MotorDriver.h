#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class MotorDriver {
public:
    MotorDriver(gpio_num_t stepPin, gpio_num_t dirPin); // Constructeur
    void setDirection(bool direction); // Définir la direction du moteur
    void stepMotor(); // Activer un pas du moteur

private:
    gpio_num_t stepPin; // Broche pour le pas du moteur
    gpio_num_t dirPin; // Broche pour la direction du moteur
    void initializePins(); // Initialiser les broches de contrôle du moteur
    esp_err_t _init(const gpio_num_t pin);

};

#endif // MOTORDRIVER_H
