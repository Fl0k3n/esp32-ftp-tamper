#ifndef FTP_COMMAND_PROCESSOR
#define FTP_COMMAND_PROCESSOR

#include <WiFi.h>

#include 

#define DATA_SERVER_PORT 50009

class FTPCommandProcessor {

public:
    FTPCommandProcessor();

private:
    WiFiServer dataServer;

};

#endif