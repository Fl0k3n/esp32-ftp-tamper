#include "KeypadModule.h"

bool KeypadModule::awaitExactInput(String exact) {

    String inputPin;
    inputPin.reserve(exact.length());
    inputPin = "";

    while (true) {
        char key = keypad.getKey();

        if (key) {
            Serial.print(key);
            if (key == '*') {
                Serial.println("Keypad input cleared");
                inputPin = "";
            }
            else if (key == '#') {
                Serial.println(inputPin);
                return inputPin == exact;
            }
            else {
                inputPin += key;
            }
        }
        vTaskDelay(1);
    }
    return false;
}


size_t KeypadModule::awaitInput(char* buff, int maxLen) {
    size_t i = 0;

    for (;i < maxLen; i++) {
        char key = keypad.getKey();

        switch (key) {
        case '*':
            i = 0;
            break;
        case '#':
            return i;
        case '\0':
            break;
        default:
            buff[i] = key;
        }

        vTaskDelay(1);
    }

    Serial.println("Max input length reached, commiting");
    return i;
}

