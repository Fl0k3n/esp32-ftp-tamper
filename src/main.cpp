#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <ChaCha.h>
#include <FTPServer.h>
#include <EMailSender.h>
#include <KeypadModule.h>
#include <LightSensor.h>

#include "config.h"
#include "cipher_key.h"

#define BAUD 9600
#define WIFI_INIT_TIMEOUT 15
#define LED_PIN 15


enum SecureMode {
    SECURE = 0,
    UNSECURE = 1
};

SecureMode secureMode;

TaskHandle_t FTPServerTask;
TaskHandle_t KeypadModuleTask;
TaskHandle_t LightSensorTask;

KeypadModule* keypadModule;

EMailSender emailSender(email, email_password);

void ftpServerTask(void*) {
    Serial.println("Starting FTPServer...");
    AccessControler accessControler;
    FTPDataProcessor dataProcessor(cipherKey, &accessControler);

    if (!dataProcessor.assertValidCipherConfig()) {
        Serial.println("Invalid cipher confing. FTPServer launching aborted.");
        vTaskDelete(NULL);
    }

    AccessControlHandler accessControlHandler(ftp_username, ftp_password);
    FTPServiceHandler ftpServiceHandler(&dataProcessor, &accessControler);
    TransferParametersHandler transferParametersHandler;

    FTPServer ftpServer(&accessControlHandler, &ftpServiceHandler, &transferParametersHandler);
    ftpServer.run();
}

void keypadModuleTask(void*) {
    Serial.println("To switch device mode between secure and unsecure, enter pin anytime.");
    while (keypadModule->enterPin()) {
        if (secureMode == SECURE) {
            Serial.println("Changed device mode to unsecure");
            digitalWrite(LED_PIN, LOW);
            secureMode = UNSECURE;
        }
        else {
            Serial.println("Changed device mode to secure");
            digitalWrite(LED_PIN, HIGH);
            secureMode = SECURE;
        }
    }
    Serial.println("Entered wrong pin too many times. Restarting the device...");
    ESP.restart(); // just for now, dont know how to really handle this
    vTaskDelete(NULL);
}

void lightSensorTask(void*) {
    LightSensor lightSensor = LightSensor();
    lightSensor.run();
    vTaskDelete(NULL);
}

void initSD() {
    if (SD.begin())
        Serial.println("SD card successfully initialized");
    else {
        Serial.println("Error: SD card initialization failed. Try to fix the error and restart the device");
        while (true) {}
    }

    Serial.println("creating test file at /test.txt");
    File f = SD.open("/test.txt", "w");
    if (!f) {
        Serial.println("failed to open");
        return;
    }

    char msg[] = "content";
    int written = f.print(msg);

    Serial.printf("written: %d\n", written);

    f.close();

    f = SD.open("/test.txt", "r");

    String content = f.readString();
    Serial.print("Got ");
    Serial.println(content);
    f.close();
}

void initKeypadModule() {
    keypadModule = new KeypadModule(pin, 8);
    Serial.println("Enter pin: ");
    if (!keypadModule->enterPin()) {
        Serial.println("ENTERED WRONG PIN TOO MANY TIMES. ABORTING LAUCHING THE DEVICE");
        while (true) {}
    }
    Serial.println("Entering secure mode");
    secureMode = SECURE;
    digitalWrite(LED_PIN, HIGH);
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

void sendWarningMails(String message) {
    EMailSender::EMailMessage msg;
    msg.subject = "TAMPER WARNING - ESP32 FTP SERVER";
    msg.message = message;

    EMailSender::Response response = emailSender.send(emails_to_notify, sizeof(emails_to_notify) / sizeof(emails_to_notify[0]), msg);
    Serial.println("Sending warning mails status: ");
    Serial.println("code: " + response.code);
    Serial.println("desc: " + response.desc);
}

void setup() {
    Serial.begin(BAUD);
    while (!Serial) {

    }

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    initSD();
    // initKeypadModule();
    connectToWiFi();

    // sendWarningMails("Warning here"); // tested - everything is correct

    xTaskCreatePinnedToCore(
        ftpServerTask, /* Task function. */
        "FTPServer",     /* name of task. */
        8192,    /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &FTPServerTask,  /* Task handle to keep track of created task */
        1          /* Core where the task should run */
    );

    // xTaskCreatePinnedToCore(
    //   keypadModuleTask,
    //   "KeypadModuleTask",
    //   8192,
    //   NULL,
    //   1,
    //   &KeypadModuleTask,
    //   0
    // );

    // xTaskCreatePinnedToCore(
    //   lightSensorTask,
    //   "LightSensorTask",
    //   8192,
    //   NULL,
    //   1,
    //   &LightSensorTask,
    //   0
    // );
}

void loop() {
    vTaskDelay(1);
}