#include <FTPServer.h>

FTPServer::FTPServer() {
    this->commandProcessor = FTPCommandProcessor();
    this->controlProcess = FTPControlProcess(this->commandProcessor);
}

void FTPServer::mainFtpLoop() {

}