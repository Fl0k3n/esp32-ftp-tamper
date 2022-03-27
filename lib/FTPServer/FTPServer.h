#ifndef FTP_SERVER
#define FTP_SERVER

#include <WiFi.h>

#include "ControlProcess/FTPControlProcess.h"
#include "CommandProcessor/FTPCommandProcessor.h"

class FTPServer {

public:
    FTPServer();
    void mainFtpLoop();

private:
    FTPControlProcess controlProcess;
    FTPCommandProcessor commandProcessor;
};

#endif