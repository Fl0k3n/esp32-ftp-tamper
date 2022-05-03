#include "TamperGuard.h"


TamperGuard::TamperGuard(EmailService* emailService)
    : emailService(emailService), mode(UNSECURE), invalidPinRetries(0) {

}

void TamperGuard::registerIntrusion(Intrusion intrusion) {
    if (xQueueSend(intrusionQueue, (void*)&intrusion, 0) != pdTRUE) {
        Serial.print("Intrusion queue full, dropping message from ");
        Serial.println(intrusion.detector);
    }
}

void TamperGuard::run() {
    if ((intrusionQueue = xQueueCreate(sizeof(Intrusion), INTRUSION_QUEUE_SIZE)) == NULL) {
        Serial.println("Failed to create intrusion queue");
        // TODO clear sensitive data and abort every other task or try to reset
        vTaskDelete(NULL);
    }

    while (true) {
        Intrusion intrusion;
        if (xQueueReceive(intrusionQueue, &intrusion, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        handleIntrusion(intrusion);
    }
}

void TamperGuard::handleIntrusion(Intrusion intrusion) {
    Serial.print("Handling intrusion");

    // TODO handle in secure
    if (mode == SECURE && intrusion.detector == KEYPAD) {
        if ((int)intrusion.sensorData == CORRECT_PIN) {
            mode = UNSECURE;
            // TODO cancel scheduled task which sets this automatically
            Serial.println("Switching to unsecure mode");
        }
        else {
            // TODO ????
            Serial.println("Invalid pin");
        }

        return;
    }

    if (mode == SECURE)
        return;

    switch (intrusion.detector)
    {
    case MOTION:
        Serial.println("Motion sensor");
        break;
    case LIGHT:
        Serial.println("Light sensor");
        break;
    case KEYPAD:
        Serial.println("Keypad");
        handleKeypadIntrusion(intrusion);
        break;
    default:
        Serial.printf("Undefined sensor %d\n", intrusion.detector);
    }

    // emailService->sendEmail("TAMPER WARNING - ESP32 FTP SERVER", "check this out");
}


void TamperGuard::handleKeypadIntrusion(Intrusion intrusion) {
    int keypadData = (int)intrusion.sensorData;

    if (keypadData == CORRECT_PIN) {
        Serial.println("Pin entered correctly, switching to secure mode");
        mode = SECURE;
        invalidPinRetries = 0;
        // TODO schedule task that resets it
    }
    else {
        Serial.printf("Got %d from keypad\n", keypadData);
        invalidPinRetries++;

        if (invalidPinRetries == MAX_PIN_RETRIES) {
            Serial.println("Limit reached");
            // TODO double waiting time or smth
        }
    }
}