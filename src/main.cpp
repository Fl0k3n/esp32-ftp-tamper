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
#include "config.h"
#include "cipher_key.h"

#define BAUD 115200
#define WIFI_INIT_TIMEOUT 15
#define LED_PIN 15

TaskHandle_t tamperGuardTaskHandle;
TaskHandle_t FTPServerTask;
TaskHandle_t KeypadModuleTask;
TaskHandle_t LightSensorTask;
TaskHandle_t movementDetectionTaskHandle;

TamperGuard* tamperGuard;

void ftpServerTask(void*) {
    Serial.println("Starting FTPServer...");
    FTPDataProcessor dataProcessor(cipherKey);

    if (!dataProcessor.assertValidCipherConfig()) {
        Serial.println("Invalid cipher confing. FTPServer launching aborted.");
        vTaskDelete(NULL);
    }

    AccessControlHandler accessControlHandler(ftp_username, ftp_password);
    FTPServiceHandler ftpServiceHandler(&dataProcessor);
    TransferParametersHandler transferParametersHandler;

    FTPServer ftpServer(&accessControlHandler, &ftpServiceHandler, &transferParametersHandler);
    ftpServer.run();
}


void tamperGuardTask(void*) {
    // TODO emails count
    EmailService emailService(email, email_password, emails_to_notify, 1);
    TamperGuard guard(&emailService);
    tamperGuard = &guard;
    tamperGuard->run();
}

void keypadModuleTask(void*) {
    Serial.println("To switch device mode between secure and unsecure, enter pin anytime.");
    KeypadModule keypadModule(pin, 8);

    while (true) {
        bool isCorrect = keypadModule.awaitPin();

        tamperGuard->registerIntrusion({
            .detector = KEYPAD,
            .timestamp = xTaskGetTickCount(),
            .sensorData = (void*)(isCorrect ? CORRECT_PIN : INCORRECT_PIN)
            });
    }
}

void lightSensorTask(void*) {
    LightSensor lightSensor = LightSensor();
    // TODO
    lightSensor.run();
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

void initSD() {
    if (SD.begin())
        Serial.println("SD card successfully initialized");
    else {
        Serial.println("Error: SD card initialization failed. Try to fix the error and restart the device");
        while (true) {}
    }
}

void checkWiFiConnectionTimeout(int* tries) {
    (*tries)++;
    if (*tries == WIFI_INIT_TIMEOUT) {
        Serial.println("\nError: Unable to connect to WiFi with given ssid and password. Try to fix the error and restart the device");
        while (true) {}
    }
}

void connectToWiFi() {
    Serial.print("Trying to connect to WiFi...");
    WiFi.begin(ssid, wifi_password);

    int tries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        checkWiFiConnectionTimeout(&tries);
        Serial.print(".");
    }

    Serial.print("\nConnected to WiFi with IP Address: \t");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(BAUD);
    while (!Serial) {

    }

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    initSD();
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

    // xTaskCreatePinnedToCore(
    //     lightSensorTask,
    //     "LightSensorTask",
    //     8192,
    //     NULL,
    //     1,
    //     &LightSensorTask,
    //     0
    // );

    xTaskCreatePinnedToCore(
        movementDetectionTask,
        "movementDetectionTask",
        8192,
        NULL,
        1,
        &movementDetectionTaskHandle,
        0);

}


void loop() {
    // Serial.println("loop");
    vTaskDelay(1000);
}