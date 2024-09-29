//interthread.cpp

#include "interthread.h"

Semaphore::Semaphore() {
    semaphore = xSemaphoreCreateBinary();
}

void Semaphore::give() {
    xSemaphoreGive(semaphore);
}

void Semaphore::take() {
    xSemaphoreTake(semaphore, portMAX_DELAY);
}

Semaphore::~Semaphore() {
    vSemaphoreDelete(semaphore);
}

/*
// Fonction exécutée par le premier thread
void thread1Function(void *param) {
    Semaphore *sem = (Semaphore *)param;
    while (true) {
        printf("Thread 1: Attendre le sémaphore\n");
        sem->take();
        printf("Thread 1: Sémaphore reçu\n");
        // Logique du thread 1
    }
}

// Fonction exécutée par le second thread
void thread2Function(void *param) {
    Semaphore *sem = (Semaphore *)param;
    while (true) {
        printf("Thread 2: Libérer le sémaphore\n");
        sem->give();
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Attendre 5 secondes
    }
}

extern "C" void app_main() {
    Semaphore sem;

    xTaskCreate(&thread1Function, "Thread 1", 2048, &sem, 5, NULL);
    xTaskCreate(&thread2Function, "Thread 2", 2048, &sem, 5, NULL);
}
*/