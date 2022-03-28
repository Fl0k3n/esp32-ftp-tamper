#ifndef FTP_SERVICE_HANDLER
#define FTP_SERVICE_HANDLER

#include <Arduino.h>

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "Session.h"

#define DATA_SERVER_PORT 50009


class FTPServiceHandler {
public:
    FTPServiceHandler();
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
};

#endif