#ifndef INTERTHREAD_H
#define INTERTHREAD_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

class Semaphore {
private:
    SemaphoreHandle_t semaphore;

public:
    Semaphore();
    void give();
    void take();
    ~Semaphore();
};

#endif // INTERTHREAD_H
