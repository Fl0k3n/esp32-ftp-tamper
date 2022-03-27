#include "FTPCommandProcessor.h"

FTPCommandProcessor::FTPCommandProcessor() {
    this->dataServer = WiFiServer(DATA_SERVER_PORT);
    this->dataServer.begin();
}