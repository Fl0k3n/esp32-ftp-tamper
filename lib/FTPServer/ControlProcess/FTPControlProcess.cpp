#include "FTPControlProcess.h"

FTPControlProcess::FTPControlProcess(FTPCommandProcessor commandProcessor) {
    this->commandProcessor = commandProcessor;

    this->controlServer = WiFiServer(CONTROL_SERVER_PORT);
    this->controlServer.begin();
}