
#ifndef OUTPUTTTL_H
#define OUTPUTTTL_H

#include "CPPGpio.h"

class OutPutTTL
{
public:
    OutPutTTL(gpio_num_t pin);

    ~OutPutTTL(void);
    StandardGpio::GpioOutput sortiTTL;

private:
    gpio_num_t _pin;

};

#endif //OUTPUTTTL_H