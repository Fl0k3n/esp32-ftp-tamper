#ifndef TRANSFER_PARAMETERS_HANDLER
#define TRANSFER_PARAMETERS_HANDLER

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "FTPCommandHandler.h"
#include "Session.h"


class TransferParametersHandler : public FTPCommandHandler {
private:
    static const String canHandleCmds[];
    static const int canHandleCmdsNumber;

    void handlePortCmd(CommandMessage*, Session*);
    void handlePasvCmd(CommandMessage*, Session*);
    void handleTypeCmd(CommandMessage*, Session*);
    void handleStruCmd(CommandMessage*, Session*);
    void handleModeCmd(CommandMessage*, Session*);

public:
    TransferParametersHandler();
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
};

#endif