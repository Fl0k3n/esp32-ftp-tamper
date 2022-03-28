#ifndef ACCESS_CONTROL_HANDLER
#define ACCESS_CONTROL_HANDLER

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "Session.h"

class AccessControlHandler {
private:
    String username;
    String password;

public:
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);

    AccessControlHandler(String username, String password);
};

#endif