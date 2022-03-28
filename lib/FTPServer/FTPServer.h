#ifndef FTP_SERVER
#define FTP_SERVER

#include <WiFi.h>

#include "ControlProcess/FTPControlProcess.h"
#include "CommandProcessor/FTPCommandProcessor.h"

class FTPServer {
private:
    WiFiServer* serverSocket;

    void acceptConnection();
    static void handleConnection(void* rtosArgs);

public:
    FTPServer();
    void run();
};

#endif