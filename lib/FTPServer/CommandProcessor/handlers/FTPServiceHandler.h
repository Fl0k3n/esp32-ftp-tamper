#ifndef FTP_SERVICE_HANDLER
#define FTP_SERVICE_HANDLER

#include <Arduino.h>

class FTPServiceHandler {

public:
    FTPServiceHandler();

private:
    String currentWorkingDirectory;
    String filePath;
};

#endif