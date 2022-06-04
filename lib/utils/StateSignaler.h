#ifndef STATE_SIGNALER
#define STATE_SIGNALER

#include <Arduino.h>

#define GREEN_LED_PIN 4
#define RED_LED_PIN 2

class StateSignaler {
public:
    void init() {
        // DONT use Serial here
        pinMode(GREEN_LED_PIN, OUTPUT);
        pinMode(RED_LED_PIN, OUTPUT);
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
    }

    void signalSecureModeEntered() {
        digitalWrite(GREEN_LED_PIN, HIGH);
    }

    void signalUnsecureModeEntered() {
        digitalWrite(GREEN_LED_PIN, LOW);
    }

    void signalSetupFinished() {
        Serial.println("setup done");
        digitalWrite(RED_LED_PIN, LOW);
    }

    void signalInfiniteLoopEntered() {
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
        Serial.println("entering infinite loop");
    }
};

#endif