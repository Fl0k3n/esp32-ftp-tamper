#ifndef TRANSFER_PARAMETERS_HANDLER
#define TRANSFER_PARAMETERS_HANDLER

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "FTPCommandHandler.h"
#include "Session.h"


class TransferParametersHandler : public FTPCommandHandler {
private:
    static const String canHandleCmds[];
    static const int canHandleCmdsNumber = 5;


    void handlePortCmd(CommandMessage*, Session*);
    void handlePasvCmd(CommandMessage*, Session*);

public:
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
};

#endif