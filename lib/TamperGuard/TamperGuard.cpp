#include "TamperGuard.h"


TamperGuard::TamperGuard(EmailService* emailService, PreferencesHandler* prefs, StateSignaler* stateSignaler,
    xTaskHandle** tasksToStop, int taskCount)
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
            handleKeypadInputInSecureMode((char*) intrusion.sensorData);
        }

        if (intrusion.detector != TIMER) {
            timeoutTask = timeoutHandler.resetTimeout(
                timeoutTask, &timeoutArgs);
        } 
        else {
            Serial.println("Secure mode timed out");
            switchToUnsecureMode(false);
        }
    }
    else {
        switch (intrusion.detector)
        {
        case MOTION:
            Serial.println("motion sensor detected movement");
            handleSecurityBreach();
            break;
        case LIGHT:
            Serial.println("light sensor detected light");
            handleSecurityBreach();
            break;
        case KEYPAD:
            handleKeypadIntrusion(intrusion);
            break;
        default:
            Serial.printf("Undefined sensor %d\n", intrusion.detector);
        }
    }
}


void TamperGuard::handleKeypadIntrusion(Intrusion intrusion) {
    char* keypadInput = (char*)intrusion.sensorData;

    if (isPinCorrect(keypadInput)) {
        switchToSecureMode();
    }
    else {
        Serial.printf("Got %s from keypad\n", keypadInput);
        invalidPinRetries++;

        if (invalidPinRetries == MAX_PIN_RETRIES) {
            Serial.println("Limit reached");
            handleSecurityBreach();
        }
    }
}

void TamperGuard::handleSecurityBreach() {
    prefs->clearSecrets();
    emailService->sendEmail("TAMPER WARNING - ESP32 FTP SERVER", "Intrusion detected!");

    stopAll();
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

bool TamperGuard::isPinCorrect(char* keypadInput) {
    return prefs->pin.equals(keypadInput);
}

void TamperGuard::handleKeypadInputInSecureMode(char* keypadInput) {
    if (isPinCorrect(keypadInput)) {
        switchToUnsecureMode(true);
    }
    else if (strcmp("A", keypadInput) == 0) {
        prefs->printPrefs();
    }
    else if (strcmp("DDDD", keypadInput) == 0) {
        prefs->flushConfig();
        Serial.println("Config removed, restarting the device...");
        ESP.restart();
    }
    else if (strcmp("ABCD", keypadInput) == 0) {
        Serial.println("restarting the device...");
        ESP.restart();
    }
    else {
        Serial.println("Unexpected keypad input: " + String(keypadInput));
    }

}
void TamperGuard::stopAll() {
    for (int i = 0; i < tasksToStopCount; i++) {
        vTaskSuspend(*tasksToStop[i]);
    }

    signaler->signalInfiniteLoopEntered();
    vTaskDelete(NULL);
}