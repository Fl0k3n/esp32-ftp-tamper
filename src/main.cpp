#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>

#include <FTPServer.h>

#include "config.h"

#define BAUD 9600
#define WIFI_INIT_TIMEOUT 15

TaskHandle_t FTPServerTask;

void ftpServerTask(void*) {
    FTPServer ftpServer = FTPServer();
    while (1) {
        ftpServer.mainFtpLoop();
    }
}

void initSD() {
    if (SD.begin())
        Serial.println("SD card successfully initialized");
    else {
        Serial.println("Error: SD card initialization failed");
        loop(); // wanted to do something to jump to the loop and stop the server from continuing initialization, but doesnt work this way
    }
}

void checkWiFiConnectionTimeout(int* tries) {
    *tries++;
    if (*tries == WIFI_INIT_TIMEOUT) {
        Serial.println("\nError: Unable to connect to WiFi with given ssid and password");
        loop(); // wanted to do something to jump to the loop and stop the server from continuing initialization, but doesnt work this way
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

    xTaskCreatePinnedToCore(
        ftpServerTask,          //Function to implement the task 
        "FTPServer",            //Name of the task
        6000,                   //Stack size in words 
        NULL,                   //Task input parameter 
        0,                      //Priority of the task 
        &FTPServerTask,         //Task handle.
        1);                     //Core where the task should run 
}

void loop() {
    // put your main code here, to run repeatedly:
}