#ifndef ACCESS_CONTROL_HANDLER
#define ACCESS_CONTROL_HANDLER

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "Session.h"
#include "FTPCommandHandler.h"

class AccessControlHandler : public FTPCommandHandler {
private:
    String username;
    String password;
    static const String canHandleCmds[];
    static const int canHandleCmdsNumber = 7;

    String changeDirectory(String, String);

    void handleAuthCmd(CommandMessage*, Session*);
    // RFC 959 - Section 4.1.1. ACCESS CONTROL COMMANDS implementation
    void handleUserCmd(CommandMessage*, Session*);
    void handlePasswordCmd(CommandMessage*, Session*);
    void handleQuitCmd(CommandMessage*, Session*);
    void handleReinitCmd(CommandMessage*, Session*);
    void handleCwdCmd(CommandMessage*, Session*);
    void handleCdupCmd(CommandMessage*, Session*);

public:
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);

    AccessControlHandler(String username, String password);
};

#endif