#ifndef FTP_CONTROL_PROCESS
#define FTP_CONTROL_PROCESS

#include <WiFi.h>

#include "../CommandProcessor/FTPCommandProcessor.h"

#define CONTROL_SERVER_PORT 21

class FTPControlProcess {

public:
    FTPControlProcess(FTPCommandProcessor = FTPCommandProcessor());

private:
    WiFiServer controlServer;
    FTPCommandProcessor commandProcessor;
};

#endif