#ifndef STATE_SIGNALER
#define STATE_SIGNALER

#include <Arduino.h>

#define LED_PIN 2

class StateSignaler {
public:
    void init() {
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, HIGH);
    }

    void signalSecureModeEntered() {
        digitalWrite(LED_PIN, LOW);
    }

    void signalUnsecureModeEntered() {
        digitalWrite(LED_PIN, HIGH);
    }

    void signalInfiniteLoopEntered() {
        // TODO
        Serial.println("entering infinite loop");
    }
};

#endif