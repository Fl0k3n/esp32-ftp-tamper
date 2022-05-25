#include "TamperGuard.h"


TamperGuard::TamperGuard(EmailService* emailService, PreferencesHandler* prefs, StateSignaler* stateSignaler,
    xTaskHandle* tasksToStop, int taskCount)
    : emailService(emailService), prefs(prefs), signaler(stateSignaler),
    tasksToStop(tasksToStop), tasksToStopCount(taskCount), mode(UNSECURE), invalidPinRetries(0) {

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
        prefs->clearSecrets();
        vTaskDelete(NULL);
    }
    
    timeoutArgs = {
        .callbackQueue = intrusionQueue,
        .callbackArgs = (void*) &TIMER_INTRUSION,
        .timeoutMs = SECURE_MODE_TIMEOUT_MS
    };

    while (true) {
        Intrusion intrusion;
        if (xQueueReceive(intrusionQueue, &intrusion, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        handleIntrusion(intrusion);
    }
}

void TamperGuard::handleIntrusion(Intrusion intrusion) {
    Serial.println("Handling intrusion");

    if (mode == SECURE) {
        if (intrusion.detector == KEYPAD) {
            if ((int)intrusion.sensorData == CORRECT_PIN) {
                switchToUnsecureMode(true);
            }
            else {
                // no idea what should we do here
                Serial.println("Invalid pin");
            }
        }
        else if (intrusion.detector != TIMER) {
            timeoutTask = timeoutHandler.resetTimeout(
                timeoutTask, &timeoutArgs);
        }
        else {
            Serial.println("Secure mode timed out");
            switchToUnsecureMode(false);
        }

        return;
    }


    switch (intrusion.detector)
    {
    case MOTION:
        Serial.println("motion sensor detected movement");
        // handleSecurityBreach();
        break;
    case LIGHT:
        Serial.println("light sensor detected light");
        // handleSecurityBreach();
        break;
    case KEYPAD:
        handleKeypadIntrusion(intrusion);
        break;
    default:
        Serial.printf("Undefined sensor %d\n", intrusion.detector);
    }
}


void TamperGuard::handleKeypadIntrusion(Intrusion intrusion) {
    int keypadData = (int)intrusion.sensorData;

    if (keypadData == CORRECT_PIN) {
        switchToSecureMode();
    }
    else {
        Serial.printf("Got %d from keypad\n", keypadData);
        invalidPinRetries++;

        if (invalidPinRetries == MAX_PIN_RETRIES) {
            Serial.println("Limit reached");
            handleSecurityBreach();
        }
    }
}

void TamperGuard::handleSecurityBreach() {
    prefs->clearSecrets();
    emailService->sendEmail("TAMPER WARNING - ESP32 FTP SERVER", "check this out");

    for (int i = 0; i < tasksToStopCount; i++) {
        vTaskSuspend(tasksToStop[i]);
    }

    signaler->signalInfiniteLoopEntered();
    while (true) {

    }
}


void TamperGuard::switchToSecureMode() {
    Serial.println("Switching to secure mode");
    signaler->signalSecureModeEntered();
    mode = SECURE;
    invalidPinRetries = 0;
    timeoutTask = timeoutHandler.requestTimeout(&timeoutArgs);
}


void TamperGuard::switchToUnsecureMode(bool shouldStopTimer) {
    Serial.println("Switching to unsecure mode");
    signaler->signalUnsecureModeEntered();
    mode = UNSECURE;
    if (shouldStopTimer) {
        timeoutHandler.cancelTimeout(timeoutTask);
    }
}