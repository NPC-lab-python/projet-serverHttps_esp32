#ifndef SERVOSCS15_H
#define SERVOSCS15_H

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "TaskNotifier.h"

class ServoSCS15 {
public:
    // Constructeur pour initialiser l'UART
    ServoSCS15(uart_port_t uart_num = UART_NUM_1, int tx_pin = 17, int rx_pin= 16, int dir_pin= 2 ,int baud_rate = 1000000);

    // Méthode pour initialiser l'UART
    void init();

    TaskHandle_t getTaskHandle();

    // Méthode pour changer l'ID d'un moteur
    void change_motor_id(uint8_t current_id, uint8_t new_id);

    // Méthode pour Sauvegarder les paramètres
    void save_servo_settings(uint8_t current_id);   

    // Méthode pour envoyer une commande UART
    void send_command(const uint8_t *data, size_t len);

    // Méthode pour pinguer un moteur
    bool ping_motor(uint8_t id);

    // Méthode pour déplacer un moteur et obtenir sa position après le mouvement
    int move_and_get_position(uint8_t id, uint16_t position, uint16_t speed);

    // Méthode pour déplacer un moteur
    void move_motor_only(uint8_t id, uint16_t position, uint16_t speed);
    
    // Méthode pour démarrer un thread de mouvement pour un moteur
    bool start_motor_thread(uint8_t id, uint16_t initial_position, uint16_t initial_speed, const char* task_name);

    bool start_motor_move_only_thread(uint8_t id, uint16_t initial_position, uint16_t initial_speed, const char* task_name);


private:
    TaskHandle_t taskHandle;
    uart_port_t uart_num_;
    int tx_pin_;
    int rx_pin_;
    int dir_pin_;
    int baud_rate_;
    SemaphoreHandle_t uart_mutex_;  // Mutex pour protéger l'accès à l'UART

    // Méthode privée pour la tâche de contrôle du moteur
    static void motor_task(void *pvParameters);
    static void motor_move_only_task(void *pvParameters);

};

#endif
