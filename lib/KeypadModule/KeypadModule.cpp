#include "KeypadModule.h"

bool KeypadModule::awaitExactInput(String exact) {
    String input;
    input.reserve(exact.length());
    input = "";

    while (true) {
        char key = keypad.getKey();

        if (key) {
            Serial.print(key);
            if (key == '*') {
                Serial.println("Keypad input cleared");
                input = "";
            }
            else if (key == '#') {
                Serial.println(input);
                return input == exact;
            }
            else {
                input += key;
            }
        }
        vTaskDelay(1);
    }
    return false;
}


size_t KeypadModule::awaitInput(char* buff, int maxLen) {
    size_t i = 0;

    for (;i < maxLen;) {
        char key = keypad.getKey();
        if (key) {
            Serial.print(key);
            switch (key) {
            case '*':
                i = 0;
                break;
            case '#':
                return i;
            default:
                buff[i] = key;
                i++;
            }
        }

        vTaskDelay(1);
    }

    Serial.println("Max input length reached, commiting");
    return i;
}

