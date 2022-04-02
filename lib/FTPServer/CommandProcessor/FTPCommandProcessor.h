#ifndef FTP_COMMAND_PROCESSOR
#define FTP_COMMAND_PROCESSOR

#include <WiFi.h>
#include "Session.h"
#include "CommandMessage.h"
#include "handlers/FTPServiceHandler.h"
#include "handlers/TransferParametersHandler.h"
#include "handlers/AccessControlHandler.h"


class FTPCommandProcessor {
private:
    Session* session;
    FTPDataProcessor* dataProcessor;
    AccessControlHandler* accessControlHandler;
    FTPServiceHandler* ftpServiceHandler;
    TransferParametersHandler* transferParametersHandler;

    void handleDisconnected();

    bool processSocketInput();
    void handleMessage();

    void checkDataConnections();

public:
    FTPCommandProcessor(Session*, AccessControlHandler*, FTPServiceHandler*, TransferParametersHandler*);
    void listenForCommands();
};

#endif