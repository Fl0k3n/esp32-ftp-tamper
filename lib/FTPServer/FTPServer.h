#ifndef FTP_SERVER
#define FTP_SERVER

#include <WiFi.h>

#include "CommandProcessor/FTPCommandProcessor.h"
#include "CommandProcessor/handlers/AccessControlHandler.h"
#include "ftpconf.h"

typedef struct CommandProcessorParams {
    WiFiServer* serverSocket;
    AccessControlHandler* accessControlHandler;
    FTPServiceHandler* ftpServiceHandler;
    TransferParametersHandler* transferParametersHandler;
} CommandProcessorParams;


class FTPServer {
private:
    WiFiServer serverSocket;

    AccessControlHandler* accessControlHandler;
    FTPServiceHandler* ftpServiceHandler;
    TransferParametersHandler* transferParametersHandler;

    void acceptConnection(CommandProcessorParams*);

    // can't pass 'this' to rtos task
    static void handleConnection(void* rtosArgs); // cast to CommandProcessorParams

public:
    // handlers should be configured externally and injected here, i assume that they will be singletons
    // and if so, they should be thread safe (if we ever will handle multiple connections)
    FTPServer(AccessControlHandler*, FTPServiceHandler*, TransferParametersHandler*);
    void run();
};



#endif