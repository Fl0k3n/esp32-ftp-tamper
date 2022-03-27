#ifndef FTP_COMMAND_PROCESSOR
#define FTP_COMMAND_PROCESSOR

#include <WiFi.h>

#include "handlers/FTPServiceHandler.h"
#include "handlers/AccessControlHandler.h"
#include "handlers/TransferParametersHandler.h"

#define DATA_SERVER_PORT 50009

class FTPCommandProcessor {

public:
    FTPCommandProcessor();

private:
    WiFiServer dataServer;

};

#endif