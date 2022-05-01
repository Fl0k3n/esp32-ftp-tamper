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
        vTaskDelay(100);
    }
}


void FTPServer::acceptConnection(CommandProcessorParams* params) {
    if (serverSocket.hasClient()) {
        Serial.println("Got new connection...");

        TaskHandle_t commandProcessorHandle;

        xTaskCreatePinnedToCore(
            handleConnection, /* Task function. */
            "FTPCommandProcessor",     /* name of task. */
            16384,    /* Stack size of task */
            params,      /* parameter of the task */
            1,         /* priority of the task */
            &commandProcessorHandle,  /* Task handle to keep track of created task */
            1          /* Core where the task should run */
        );
    }
    else {
        // Serial.println("No connection...");
    }
}


void FTPServer::handleConnection(void* rtosArgs) {
    CommandProcessorParams* params = (CommandProcessorParams*)rtosArgs;
    ChaCha chacha = ChaCha(CHACHA_ROUNDS);

    WiFiClient client = params->serverSocket->available();
    Session session(&client, &chacha);


    FTPCommandProcessor commandProcessor(&session, params->accessControlHandler,
        params->ftpServiceHandler, params->transferParametersHandler);

    // blocks as long as connection is alive
    commandProcessor.listenForCommands();

    // client disconnected, we are still in separate rtos task, delete it
    vTaskDelete(NULL);
}