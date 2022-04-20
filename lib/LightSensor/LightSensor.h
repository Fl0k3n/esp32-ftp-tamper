#ifndef LIGHT_SENSOR
#define LIGHT_SENSOR

#include <Arduino.h>

#define ANALOG_PIN 36

class LightSensor {
    public:
        void run();
};

#endif