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
#define TASKS_TO_STOP_COUNT 4
#define KEYPAD_BUFF_SIZE 12

#define LIGHT_SENSOR_THRESHOLD 16
#define ACCCELERO_THRESHOLD 0.3
#define GYRO_THRESHOLD 0.2

PreferencesHandler prefs;
StateSignaler signaler;
TamperGuard* tamperGuard;

TaskHandle_t tamperGuardTaskHandle;
TaskHandle_t FTPServerTaskHandle;
TaskHandle_t keypadModuleTaskHandle;
TaskHandle_t lightSensorTaskHandle;
TaskHandle_t movementDetectionTaskHandle;

TaskHandle_t* tasksToStop[TASKS_TO_STOP_COUNT] = {
    &FTPServerTaskHandle,
    &keypadModuleTaskHandle,
    &lightSensorTaskHandle,
    &movementDetectionTaskHandle
};

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
    TamperGuard guard(&emailService, &prefs, &signaler, tasksToStop, TASKS_TO_STOP_COUNT);

    tamperGuard = &guard;
    tamperGuard->run();
}

void keypadModuleTask(void*) {
    KeypadModule keypadModule;
    char buff[KEYPAD_BUFF_SIZE+1];

    while (true) {
        size_t inputSize = keypadModule.awaitInput(buff, KEYPAD_BUFF_SIZE);

        if (inputSize > 0) {
            buff[inputSize] = '\0';
            tamperGuard->registerIntrusion({
                .detector = KEYPAD,
                .timestamp = xTaskGetTickCount(),
                .sensorData = (void*)buff
                });
        }
    }
}

void lightSensorTask(void*) {
    LightSensor lightSensor = LightSensor(LIGHT_SENSOR_THRESHOLD);
    lightSensor.calibrate();

    while (true) {
        lightSensor.processNewLightValue();

        if (lightSensor.checkForAnomaly()) {
            tamperGuard->registerIntrusion({
                .detector = LIGHT,
                .timestamp = xTaskGetTickCount()
                });

            vTaskDelay(1000 / portTICK_PERIOD_MS);
            lightSensor.calibrate();
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void movementDetectionTask(void*) {
    MotionSensor motionSensor;
    if (!motionSensor.init(ACCCELERO_THRESHOLD, GYRO_THRESHOLD)) {
        Serial.println("Failed to init motion sensor, check I2C connection");
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

void assertValidPinEnteredOnInit() {
    KeypadModule keypad;
    Serial.println("Preferences found, please enter PIN");

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
        Serial.println("Preferences cleared, aborting init...");
        infLoop();
    }
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
    else if (prefs.loadFromSDIfPresent()) {
        Serial.println("Using new config from SD");
    }

    assertValidPinEnteredOnInit();
}

void setup() {
    signaler.init();
    initSerial();
    initSD();

    delay(2000);
    initPreferences();

    connectToWiFi();

    xTaskCreatePinnedToCore(
        ftpServerTask, /* Task function. */
        "FTPServer",     /* name of task. */
        8192,    /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &FTPServerTaskHandle,  /* Task handle to keep track of created task */
        1          /* Core where the task should run */
    );

    xTaskCreatePinnedToCore(
        keypadModuleTask,
        "keypadModuleTask",
        8192,
        NULL,
        1,
        &keypadModuleTaskHandle,
        0
    );

    xTaskCreatePinnedToCore(
        lightSensorTask,
        "lightSensorTask",
        8192,
        NULL,
        2,
        &lightSensorTaskHandle,
        0
    );

    xTaskCreatePinnedToCore(
        movementDetectionTask,
        "movementDetectionTask",
        8192,
        NULL,
        2,
        &movementDetectionTaskHandle,
        0);

    // assert that this task has max priority
    xTaskCreatePinnedToCore(
        tamperGuardTask,
        "tamperGuardTask",
        8192,
        NULL,
        10,
        &tamperGuardTaskHandle,
        0);

    signaler.signalSetupFinished();
}


void loop() {
    vTaskDelay(1000);
}