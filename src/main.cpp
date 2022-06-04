#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <ChaCha.h>
#include <FTPServer.h>
#include <KeypadModule.h>
#include <LightSensor.h>
#include <MotionSensor.h>
#include <TamperGuard.h>

#include "EmailService.h"
#include "PreferencesHandler.h"
#include "StateSignaler.h"

#define BAUD 115200
#define WIFI_INIT_TIMEOUT 15
#define LED_PIN 2

PreferencesHandler prefs;
StateSignaler signaler;
TamperGuard* tamperGuard;

TaskHandle_t tamperGuardTaskHandle;
TaskHandle_t FTPServerTask;
TaskHandle_t KeypadModuleTask;
TaskHandle_t LightSensorTask;
TaskHandle_t movementDetectionTaskHandle;

void infLoop() {
    signaler.signalInfiniteLoopEntered();
    while (true) {}
}

void ftpServerTask(void*) {
    Serial.println("Starting FTPServer...");
    FTPDataProcessor dataProcessor(prefs.secretKey);

    if (!dataProcessor.assertValidCipherConfig()) {
        Serial.println("Invalid cipher confing. FTPServer launching aborted.");
        vTaskDelete(NULL);
    }

    AccessControlHandler accessControlHandler(prefs.ftpUsername.c_str(), prefs.ftpPasswd.c_str());
    FTPServiceHandler ftpServiceHandler(&dataProcessor);
    TransferParametersHandler transferParametersHandler;

    FTPServer ftpServer(&accessControlHandler, &ftpServiceHandler, &transferParametersHandler);
    ftpServer.run();
}


void tamperGuardTask(void*) {
    EmailService emailService(prefs.email.c_str(), prefs.emailPasswd.c_str(), prefs.emailToNotify.c_str());
    TamperGuard guard(&emailService, &prefs, &signaler, { &FTPServerTask }, 1);
    tamperGuard = &guard;
    tamperGuard->run();
}

void keypadModuleTask(void*) {
    KeypadModule keypadModule;

    while (true) {
        bool isCorrect = keypadModule.awaitExactInput(prefs.pin);

        tamperGuard->registerIntrusion({
            .detector = KEYPAD,
            .timestamp = xTaskGetTickCount(),
            .sensorData = (void*)(isCorrect ? CORRECT_PIN : INCORRECT_PIN)
            });
    }
}

void lightSensorTask(void*) {
    LightSensor lightSensor = LightSensor();

    while (true) {
        int val = lightSensor.readValueFromLightSensor();

        // Serial.println("Light sensor: " + String(val));

        lightSensor.processNewLightValue(val);
        if (lightSensor.checkForAnomaly()) {
            tamperGuard->registerIntrusion({
                .detector = LIGHT,
                .timestamp = xTaskGetTickCount()
                });

            lightSensor.resetAnomalies();
            lightSensor.resetLightValues();
        }

        vTaskDelay(200);
    }
    vTaskDelete(NULL);
}

void movementDetectionTask(void*) {
    MotionSensor motionSensor;
    if (!motionSensor.init(0.3, 0.2)) {
        Serial.println("Failed to init MPU, check I2C connection");
        vTaskDelete(NULL);
    }

    motionSensor.calibrate();

    while (true) {
        vTaskDelay(250 / portTICK_PERIOD_MS);

        if (motionSensor.isMovementDetected()) {
            tamperGuard->registerIntrusion({
                .detector = MOTION,
                .timestamp = xTaskGetTickCount()
                });

            motionSensor.calibrate();
        }
    }
}

void initSerial() {
    Serial.begin(BAUD);
    while (!Serial) {

    }
}

void initSD() {
    if (SD.begin())
        Serial.println("SD card successfully initialized");
    else {
        Serial.println("Error: SD card initialization failed. Try to fix the error and restart the device");
        infLoop();
    }
}

void checkWiFiConnectionTimeout(int* tries) {
    (*tries)++;
    if (*tries == WIFI_INIT_TIMEOUT) {
        Serial.println("\nError: Unable to connect to WiFi with given ssid and password. Try to fix the error and restart the device");
        infLoop();
    }
}

void connectToWiFi() {
    Serial.print("Trying to connect to WiFi...");
    WiFi.begin(prefs.ssid.c_str(), prefs.wifiPasswd.c_str());

    int tries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        checkWiFiConnectionTimeout(&tries);
        Serial.print(".");
    }

    Serial.print("\nConnected to WiFi with IP Address: \t");
    Serial.println(WiFi.localIP());
}


void initPreferences() {
    if (!prefs.begin()) {
        Serial.println("Required preferences not found, checking SD");

        if (!prefs.loadFromSDCard()) {
            Serial.println("Required preferences not found on SD card, creating config dump...");
            prefs.createConfigFileExample();
            Serial.println("Config example created at " + String(CONFIG_FILENAME) + " fix it and restart, aborting init.");

            infLoop();
        }
        else {
            Serial.println("Using new config from SD");
        }
    }
    else {
        // prefs.flushConfig();
        KeypadModule keypad;
        Serial.println("Preferences found, please enter PIN");
        prefs.printPrefs(); // TODO testing only

        while (prefs.getPinRetriesLeft() > 0) {
            if (keypad.awaitExactInput(prefs.pin)) {
                prefs.resetPinRetries();
                break;
            }
            else {
                prefs.setPinRetriesLeft(prefs.getPinRetriesLeft() - 1);
                Serial.printf("Invalid PIN, you have %d retries left\n", prefs.getPinRetriesLeft());
            }
        }

        if (prefs.getPinRetriesLeft() <= 0) {
            prefs.clearSecrets();
            prefs.printPrefs();
            Serial.println("Preferences cleared, aborting init...");
            infLoop();
        }

        if (prefs.loadFromSDIfPresent()) {
            Serial.println("Using new config from SD");
        }
    }
}

void setup() {
    signaler.init();
    initSerial();
    initSD();
    initPreferences();

    prefs.printPrefs();

    connectToWiFi();

    // assert that this task has max priority
    xTaskCreatePinnedToCore(
        tamperGuardTask,
        "tamperGuardTask",
        8192,
        NULL,
        10,
        &tamperGuardTaskHandle,
        0);


    xTaskCreatePinnedToCore(
        ftpServerTask, /* Task function. */
        "FTPServer",     /* name of task. */
        8192,    /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &FTPServerTask,  /* Task handle to keep track of created task */
        1          /* Core where the task should run */
    );

    xTaskCreatePinnedToCore(
        keypadModuleTask,
        "KeypadModuleTask",
        8192,
        NULL,
        1,
        &KeypadModuleTask,
        0
    );

    xTaskCreatePinnedToCore(
        lightSensorTask,
        "LightSensorTask",
        8192,
        NULL,
        1,
        &LightSensorTask,
        0
    );

    xTaskCreatePinnedToCore(
        movementDetectionTask,
        "movementDetectionTask",
        8192,
        NULL,
        1,
        &movementDetectionTaskHandle,
        0);

    signaler.signalSetupFinished();
}


void loop() {
    vTaskDelay(1000);
}