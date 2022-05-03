#include "KeypadModule.h"

KeypadModule::KeypadModule(const char* pin, int maxPinLength) {
    this->pin = String(pin);
    this->maxPinLength = maxPinLength;
}

bool KeypadModule::awaitPin() {

    String inputPin;
    inputPin.reserve(8);
    inputPin = "";

    while (true) {
        char key = keypad.getKey();

        if (key) {
            Serial.print(key);
            if (key == '*') {
                Serial.println("INPUT PIN CLEARED");
                inputPin = "";
            }
            else if (key == '#') {
                Serial.println(inputPin);
                return inputPin == pin;
            }
            else {
                inputPin += key;
            }
        }
        vTaskDelay(1);
    }
    return false;
}