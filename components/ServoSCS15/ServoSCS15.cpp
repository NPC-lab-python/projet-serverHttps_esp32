#include "ServoSCS15.h"

// Tag pour les logs
static const char *TAG = "ServoSCS15";

// Structure pour passer les paramètres à la tâche
struct MotorTaskParams
{
    ServoSCS15 *controller; // Référence à l'objet ServoSCS15
    uint8_t motor_id;
    uint16_t initial_position;
    uint16_t initial_speed;
};
TaskHandle_t ServoSCS15::getTaskHandle()
{
    return taskHandle;
}
// Constructeur
ServoSCS15::ServoSCS15(uart_port_t uart_num, int tx_pin, int rx_pin, int dir_pin, int baud_rate)
    : uart_num_(uart_num), tx_pin_(tx_pin), rx_pin_(rx_pin), dir_pin_(dir_pin), baud_rate_(baud_rate)
{

    uart_mutex_ = xSemaphoreCreateMutex(); // Créer un mutex pour l'UART
    if (uart_mutex_ == NULL)
    {
        ESP_LOGE(TAG, "Failed to create UART mutex");
        abort(); // Si le mutex n'est pas créé correctement, arrêter le programme
    }
}

// Méthode pour initialiser l'UART
void ServoSCS15::init()
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate_,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // Configurer l'UART
    ESP_ERROR_CHECK(uart_driver_install(uart_num_, 1024, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num_, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num_, tx_pin_, rx_pin_, dir_pin_, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(uart_num_, UART_MODE_RS485_HALF_DUPLEX));
    ESP_LOGI(TAG, "UART initialized");
}

// Méthode pour envoyer une commande via UART
void ServoSCS15::send_command(const uint8_t *data, size_t len)
{
    if (uart_mutex_ == NULL)
    {
        ESP_LOGE(TAG, "UART mutex is NULL, cannot send command");
        return;
    }

    ESP_LOGI(TAG, "Trying to take UART mutex in send_command");
    if (xSemaphoreTake(uart_mutex_, portMAX_DELAY))
    {
        ESP_LOGI(TAG, "UART mutex taken in send_command");

        // Ajout de logs pour vérifier les données avant l'envoi
        ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);

        // Envoi de la commande via l'UART
        int bytes_sent = uart_write_bytes(uart_num_, (const char *)data, len);
        if (bytes_sent == len)
        {
            ESP_LOGI(TAG, "Command sent successfully, %d bytes", bytes_sent);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send all data, only %d bytes sent", bytes_sent);
        }

        xSemaphoreGive(uart_mutex_); // Libérer le mutex après envoi
        ESP_LOGI(TAG, "UART mutex released in send_command");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to take UART mutex in send_command");
    }
}

// Méthode pour changer l'ID d'un moteur
void ServoSCS15::change_motor_id(uint8_t current_id, uint8_t new_id)
{
    //     uint8_t command[] = {
    //         0xFF, 0xFF, current_id, 0x04, 0x03, 0x05, new_id, 0x00
    //     };
    //     command[7] = ~(command[2] + command[3] + command[4] + command[5] + command[6]);
    //     send_command(command, sizeof(command));
    //     ESP_LOGI(TAG, "Changement de l'ID du moteur %d à %d", current_id, new_id);
    // }
    uint8_t command[8];
    command[0] = 0xFF;
    command[1] = 0xFF;
    command[2] = current_id;
    command[3] = 0x04;
    command[4] = 0x03;
    command[5] = 0x05;
    command[6] = new_id;

    uint8_t checksum = ~(command[2] + command[3] + command[4] + command[5] + command[6]) & 0xFF;
    command[7] = checksum;

    uart_write_bytes(uart_num_, (const char *)command, sizeof(command));
    ESP_LOGI(TAG, "Commande de changement d'ID envoyée: ID actuel %d, Nouvel ID %d", current_id, new_id);
}

// Méthode pour sauvegarder les paramètres dans l'EEPROM
void ServoSCS15::save_servo_settings(uint8_t current_id)
{
    uint8_t command[6];
    command[0] = 0xFF;
    command[1] = 0xFF;
    command[2] = current_id;
    command[3] = 0x02;
    command[4] = 0x40;

    uint8_t checksum = ~(command[2] + command[3] + command[4]) & 0xFF;
    command[5] = checksum;

    uart_write_bytes(uart_num_, (const char *)command, sizeof(command));
    ESP_LOGI(TAG, "Commande de sauvegarde dans l'EEPROM envoyée pour ID %d", current_id);
}

// Méthode pour pinguer un moteur
bool ServoSCS15::ping_motor(uint8_t id)
{
    uint8_t pingCommand[] = {0xFF, 0xFF, id, 0x02, 0x01, 0x00};
    pingCommand[5] = ~(pingCommand[2] + pingCommand[3] + pingCommand[4]);

    ESP_LOGI(TAG, "Sending ping to Motor %d", id);

    if (xSemaphoreTake(uart_mutex_, portMAX_DELAY))
    {
        // send_command(pingCommand, sizeof(pingCommand));
        uart_write_bytes(uart_num_, (const char *)pingCommand, sizeof(pingCommand));

        // Lire la réponse de l'UART
        uint8_t data[1024];
        int len = uart_read_bytes(uart_num_, data, sizeof(data), pdMS_TO_TICKS(1000));
        xSemaphoreGive(uart_mutex_); // Libérer le mutex après lecture

        if (len > 0)
        {
            ESP_LOGI(TAG, "Ping response received from Motor %d: %d bytes", id, len);
            return true;
        }
        else
        {
            ESP_LOGE(TAG, "No ping response from Motor %d", id);
            return false;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Failed to take UART mutex for ping");
        return false;
    }
}

void ServoSCS15::move_motor_only(uint8_t id, uint16_t position, uint16_t speed)
{
    if (uart_mutex_ == NULL)
    {
        ESP_LOGE(TAG, "UART mutex is NULL, cannot move motor");
        return;
    }

    uint8_t posH = (position >> 8) & 0xFF;
    uint8_t posL = position & 0xFF;
    uint8_t speedH = (speed >> 8) & 0xFF;
    uint8_t speedL = speed & 0xFF;

    uint8_t moveCommand[] = {
        0xFF, 0xFF, id, 0x07, 0x03, 0x2A, posH, posL, speedH, speedL, 0x00};
    moveCommand[10] = ~(moveCommand[2] + moveCommand[3] + moveCommand[4] + moveCommand[5] + moveCommand[6] + moveCommand[7] + moveCommand[8] + moveCommand[9]);

    ESP_LOGI(TAG, "Trying to take UART mutex for Motor %d", id);
    if (xSemaphoreTake(uart_mutex_, portMAX_DELAY))
    {
        ESP_LOGI(TAG, "UART mutex taken for Motor %d", id);
        xSemaphoreGive(uart_mutex_); // Libérer le mutex après envoi

        // uart_write_bytes(uart_num_, (const char *)moveCommand, sizeof(moveCommand));
        send_command(moveCommand, sizeof(moveCommand));
        ESP_LOGI(TAG, "Move command sent to Motor %d", id);

        vTaskDelay(pdMS_TO_TICKS(500)); // Attendre que le moteur ait commencé à bouger

        ESP_LOGI(TAG, "UART mutex released for Motor %d", id);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to take UART mutex for Motor %d", id);
    }
}

// Méthode pour déplacer un moteur et obtenir sa position après le mouvement
int ServoSCS15::move_and_get_position(uint8_t id, uint16_t position, uint16_t speed)
{
    if (uart_mutex_ == NULL)
    {
        ESP_LOGE(TAG, "UART mutex is NULL, cannot move motor");
        return -1; // Ne pas tenter de déplacer le moteur si le mutex est NULL
    }

    uint8_t posH = (position >> 8) & 0xFF;
    uint8_t posL = position & 0xFF;
    uint8_t speedH = (speed >> 8) & 0xFF;
    uint8_t speedL = speed & 0xFF;

    uint8_t moveCommand[] = {
        0xFF, 0xFF, id, 0x07, 0x03, 0x2A, posH, posL, speedH, speedL, 0x00};
    moveCommand[10] = ~(moveCommand[2] + moveCommand[3] + moveCommand[4] + moveCommand[5] + moveCommand[6] + moveCommand[7] + moveCommand[8] + moveCommand[9]);

    // Envoi de la commande de déplacement
    ESP_LOGI(TAG, "Sending move command to Motor %d", id);
    // if (xSemaphoreTake(uart_mutex_, portMAX_DELAY)) {
    send_command(moveCommand, sizeof(moveCommand));
    vTaskDelay(pdMS_TO_TICKS(500)); // Attendre que le moteur ait commencé à bouger

    // Lire la position actuelle après le mouvement
    uint8_t readPosCommand[] = {0xFF, 0xFF, id, 0x04, 0x02, 0x38, 0x02, 0x00};
    readPosCommand[7] = ~(readPosCommand[2] + readPosCommand[3] + readPosCommand[4] + readPosCommand[5] + readPosCommand[6]);

    // Envoyer la commande de lecture de position
    ESP_LOGI(TAG, "Sending read position command to Motor %d", id);
    send_command(readPosCommand, sizeof(readPosCommand));

    // Lire la réponse de l'UART
    uint8_t data[1024];
    int len = uart_read_bytes(uart_num_, data, 1024, pdMS_TO_TICKS(1000));
    xSemaphoreGive(uart_mutex_); // Libérer le mutex après lecture

    if (len > 0)
    {
        ESP_LOGI(TAG, "Data received from Motor %d: %d bytes", id, len);
        int current_position = (data[5] << 8) | data[6];
        ESP_LOGI(TAG, "Position actuelle du moteur %d : %d", id, current_position);
        return current_position;
    }
    else
    {
        ESP_LOGE(TAG, "Aucune réponse du moteur %d", id);
        return -1;
    }
    // } else {
    //     ESP_LOGE(TAG, "Failed to take UART mutex");
    //     return -1;
    // }
}

// Méthode pour la tâche de contrôle du moteur
void ServoSCS15::motor_task(void *pvParameters)
{
    MotorTaskParams *params = (MotorTaskParams *)pvParameters;

    // Vérifiez que params n'est pas NULL
    if (params == NULL || params->controller == NULL)
    {
        ESP_LOGE(TAG, "Motor task parameters are NULL");
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "Motor task for Motor ID %d started", params->motor_id);

    while (1)
    {
        // Déplacer le moteur à la position initiale et obtenir sa position
        ESP_LOGI(TAG, "Motor %d: Moving to position %d", params->motor_id, params->initial_position);
        int position = params->controller->move_and_get_position(params->motor_id, params->initial_position, params->initial_speed);
        ESP_LOGI(TAG, "Motor %d: Position after movement: %d", params->motor_id, position);

        vTaskDelay(pdMS_TO_TICKS(1000));

        // Déplacer le moteur dans une autre position
        uint16_t next_position = (params->initial_position == 0) ? 1000 : 0;
        ESP_LOGI(TAG, "Motor %d: Moving to position %d", params->motor_id, next_position);
        position = params->controller->move_and_get_position(params->motor_id, next_position, params->initial_speed);
        ESP_LOGI(TAG, "Motor %d: Position after movement: %d", params->motor_id, position);

        vTaskDelay(pdMS_TO_TICKS(2000)); // Attendre 2 secondes
    }
}

// Méthode pour la tâche de contrôle du moteur (sans lecture de position)
void ServoSCS15::motor_move_only_task(void *pvParameters)
{
    MotorTaskParams *params = (MotorTaskParams *)pvParameters;

    // Vérifiez que params n'est pas NULL
    if (params == NULL || params->controller == NULL)
    {
        ESP_LOGE(TAG, "Motor task parameters are NULL");
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "Motor task for Motor ID %d started", params->motor_id);
    TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();

    while (1)
    {
        int ulNotificationValue = TaskNotifier::waitForNotification(taskHandle);

        // ESP_LOGI(TAG, "mqtt value notification : %d", ulNotificationValue);
        // Vérifier la valeur de la notification pour déterminer l'action
        switch (ulNotificationValue)
        {
            case 1: // Code 1 pour aller à la position 1
                {ESP_LOGI(TAG, "Moteur %d: Se déplace vers la position %d", params->motor_id, params->initial_position);
                params->controller->move_motor_only(params->motor_id, params->initial_position, params->initial_speed);
                break;}
            //     // ESP_LOGI(TAG, "Déplacement vers la position initiale");
            // // Déplacer le moteur à la position initiale
            // ESP_LOGI(TAG, "Motor %d: Moving to position %d", params->motor_id, params->initial_position);
            // params->controller->move_motor_only(params->motor_id, params->initial_position, params->initial_speed);

            // vTaskDelay(pdMS_TO_TICKS(1000));  // Attendre 1 seconde

            // // Déplacer le moteur dans une autre position
            // uint16_t next_position = (params->initial_position == 0) ? 1000 : 0;
            // ESP_LOGI(TAG, "Motor %d: Moving to position %d", params->motor_id, next_position);
            // params->controller->move_motor_only(params->motor_id, next_position, params->initial_speed);

            // vTaskDelay(pdMS_TO_TICKS(2000));  // Attendre 2 secondes            break;
            case 2: // Code 2 pour retourner à la position 2
                {ESP_LOGI(TAG, "Moteur %d: Se déplace vers une autre position", params->motor_id);
                uint16_t next_position = (params->initial_position == 0) ? 1000 : 0;
                params->controller->move_motor_only(params->motor_id, next_position, params->initial_speed);
                break;}
                
            default:
                {ESP_LOGI(TAG, "Notification inconnue: %d", ulNotificationValue);
                break;}
        }
                vTaskDelay(pdMS_TO_TICKS(100));  // Attendre avant la prochaine notification

    }
}

// Méthode pour démarrer un thread pour contrôler un moteur
bool ServoSCS15::start_motor_thread(uint8_t id, uint16_t initial_position, uint16_t initial_speed, const char *task_name)
{
    MotorTaskParams *params = (MotorTaskParams *)malloc(sizeof(MotorTaskParams)); // Utiliser malloc pour l'allocation
    if (params == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for MotorTaskParams");
        return false;
    }
    params->controller = this;
    params->motor_id = id;
    params->initial_position = initial_position;
    params->initial_speed = initial_speed;

    // Créer la tâche avec une taille de pile plus grande
    if (xTaskCreate(motor_task, task_name, 4096, params, 5, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task %s", task_name);
        free(params); // Libérer la mémoire si la tâche n'a pas pu être créée
        return false;
    }

    return true; // Tâche créée avec succès
}

// Méthode pour démarrer un thread pour contrôler un moteur
bool ServoSCS15::start_motor_move_only_thread(uint8_t id, uint16_t initial_position, uint16_t initial_speed, const char *task_name)
{
    MotorTaskParams *params = (MotorTaskParams *)malloc(sizeof(MotorTaskParams)); // Utiliser malloc pour l'allocation
    if (params == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for MotorTaskParams");
        return false;
    }
    params->controller = this;
    params->motor_id = id;
    params->initial_position = initial_position;
    params->initial_speed = initial_speed;

    // Créer la tâche avec une taille de pile plus grande
    if (xTaskCreate(motor_move_only_task, task_name, 4096, params, 5, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task %s", task_name);
        free(params); // Libérer la mémoire si la tâche n'a pas pu être créée
        return false;
    }

    return true; // Tâche créée avec succès
}
