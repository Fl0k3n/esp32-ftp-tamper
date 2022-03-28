#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>

#include <FTPServer.h>

#include "config.h"

#define BAUD 9600
#define WIFI_INIT_TIMEOUT 15

TaskHandle_t FTPServerTask;

void ftpServerTask(void*) {
    Serial.println("starting server...");

    AccessControlHandler accessControlHandler(ftp_username, ftp_password);
    FTPServiceHandler ftpServiceHandler;
    TransferParametersHandler transferParametersHandler;

    FTPServer ftpServer(&accessControlHandler, &ftpServiceHandler, &transferParametersHandler);
    ftpServer.run();
}

void initSD() {
    if (SD.begin())
        Serial.println("SD card successfully initialized");
    else {
        Serial.println("Error: SD card initialization failed");
        while (true) {} // maybe we can reset the board instead of this
    }
}

void checkWiFiConnectionTimeout(int* tries) {
    (*tries)++;
    if (*tries == WIFI_INIT_TIMEOUT) {
        Serial.println("\nError: Unable to connect to WiFi with given ssid and password");
        while (true) {} // maybe we can reset the board instead of this
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

    initSD();
    connectToWiFi();

    // serial didnt work with pinned to core 1, no idea why
    xTaskCreate(
        ftpServerTask,          //Function to implement the task 
        "FTPServer",            //Name of the task
        8192,                   //Stack size in words 
        NULL,                   //Task input parameter 
        5,                      //Priority of the task 
        &FTPServerTask         //Task handle.
    );                     //Core where the task should run 
}

void loop() {
    // put your main code here, to run repeatedly:
}