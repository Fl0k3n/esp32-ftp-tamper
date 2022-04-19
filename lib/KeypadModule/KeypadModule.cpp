#include "KeypadModule.h"

KeypadModule::KeypadModule(const char* pin, int maxPinLength) {
    this->pin = String(pin);
    this->maxPinLength = maxPinLength;
}

bool KeypadModule::enterPin() {

    String inputPin;
    inputPin.reserve(8);
    inputPin = "";

    int tries = 0;

    while (tries < TRIES) {
        char key = keypad.getKey();

        if (key) {
            Serial.print(key);
            if (key == '*') {
                Serial.println();
                Serial.println("INPUT PIN CLEARED");
                inputPin = "";
            }
            else if (key == '#') {
                Serial.println();
                Serial.println(inputPin);
                if (inputPin == pin) {
                    Serial.println("CORRECT PIN. ACCESS GRANTED");
                    return true;
                }
                tries++;
                Serial.println("WRONG PIN. ACCESS DENIED \nTry again... " + String(TRIES - tries) + " tries left");
                inputPin = "";
            }
            else {
                inputPin += key;
            }
        }
        vTaskDelay(1);
    }
    return false;
}