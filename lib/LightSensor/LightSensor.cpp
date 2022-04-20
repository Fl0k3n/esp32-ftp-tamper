#include "LightSensor.h"

void LightSensor::run() {
    while (1) {
        int val = analogRead(ANALOG_PIN);
        Serial.println("Light sensor: " + String(val));
        vTaskDelay(200);
    }
}