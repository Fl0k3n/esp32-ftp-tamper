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

    String ipStringForPasv;
    uint16_t dataPort;

    char pasvResponse[24];
    void sendPasvResponse(Session*);

    void handlePortCmd(CommandMessage*, Session*);
    void handlePasvCmd(CommandMessage*, Session*);
    void handleTypeCmd(CommandMessage*, Session*);

public:
    TransferParametersHandler();
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
};

#endif