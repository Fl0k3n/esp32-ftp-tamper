#ifndef FTP_COMMAND_PROCESSOR
#define FTP_COMMAND_PROCESSOR

#include <WiFi.h>
#include "Session.h"
#include "Message.h"
#include "handlers/FTPServiceHandler.h"
#include "handlers/AccessControlHandler.h"
#include "handlers/TransferParametersHandler.h"

#define DATA_SERVER_PORT 50009

class FTPCommandProcessor {
private:
    Session* session;
    TaskHandle_t task;

    void handleDisconnected();
    bool processSocketInput();
    void handleMessage();

public:
    FTPCommandProcessor(Session*);
    void listenForCommands();
};

#endif