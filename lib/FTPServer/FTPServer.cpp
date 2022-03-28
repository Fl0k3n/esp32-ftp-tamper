#include <FTPServer.h>

FTPServer::FTPServer(AccessControlHandler* accessControlHandler,
    FTPServiceHandler* ftpServiceHandler,
    TransferParametersHandler* transferParametersHandler)
    : serverSocket(WiFiServer(CONTROL_SERVER_PORT)), accessControlHandler(accessControlHandler),
    ftpServiceHandler(ftpServiceHandler), transferParametersHandler(transferParametersHandler) {}


// this thread accepts new connection(s) and creates another thread(s) to handle communication
void FTPServer::run() {
    serverSocket.begin();
    Serial.println("FTP server running");


    CommandProcessorParams params = {
        .serverSocket = &serverSocket,
        .accessControlHandler = accessControlHandler,
        .ftpServiceHandler = ftpServiceHandler,
        .transferParametersHandler = transferParametersHandler
    };

    while (true) {
        acceptConnection(&params);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void FTPServer::acceptConnection(CommandProcessorParams* params) {
    if (serverSocket.hasClient()) {
        Serial.println("Got new connection...");

        TaskHandle_t commandProcessorHandle;

        xTaskCreate(
            handleConnection,
            "FTPCommandProcessor",
            8192,
            params,
            1,
            &commandProcessorHandle);
    }
    else {
        // Serial.println("No connection...");
    }
}


void FTPServer::handleConnection(void* rtosArgs) {
    CommandProcessorParams* params = (CommandProcessorParams*)rtosArgs;

    WiFiClient client = params->serverSocket->available();
    Session session(&client);

    FTPCommandProcessor commandProcessor(&session, params->accessControlHandler,
        params->ftpServiceHandler, params->transferParametersHandler);

    // blocks as long as connection is alive
    commandProcessor.listenForCommands();

    // client disconnected, we are still in separate rtos task, delete it
    vTaskDelete(NULL);
}