#include "FTPControlProcess.h"

FTPControlProcess::FTPControlProcess(FTPCommandProcessor* commandProcessor) : commandProcessor(commandProcessor) {
    controlServer = new WiFiServer(CONTROL_SERVER_PORT);
    controlServer->begin();

    Serial.printf("Control Processor listening at %d\n", CONTROL_SERVER_PORT);
}

FTPControlProcess::~FTPControlProcess() {
    delete controlServer;
}