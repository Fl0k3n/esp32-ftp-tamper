#include <FTPServer.h>

FTPServer::FTPServer() {
    serverSocket = new WiFiServer(CONTROL_SERVER_PORT);
}

void FTPServer::run() {
    serverSocket->begin();
    Serial.println("FTP server running");

    while (true) {
        Serial.println("prrt");
        acceptConnection();

        vTaskDelay(1000 / portTICK_PERIOD_MS); //idk
    }
}


void FTPServer::acceptConnection() {
    if (serverSocket->hasClient()) {
        Serial.println("Got new connection...");

        TaskHandle_t commandProcessorHandle;

        xTaskCreate(
            handleConnection,
            "FTPCommandProcessor",
            8192,
            serverSocket,
            1,
            &commandProcessorHandle);
    }
}


void FTPServer::handleConnection(void* rtosArgs) {
    WiFiServer* serverSocket = (WiFiServer*)rtosArgs;
    WiFiClient client = serverSocket->available();
    Session* session = new Session(&client);

    FTPCommandProcessor commandProcessor = FTPCommandProcessor(session);

    // blocks as long as connection is alive
    commandProcessor.listenForCommands();
}