#ifndef TRANSFER_PARAMETERS_HANDLER
#define TRANSFER_PARAMETERS_HANDLER

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "Session.h"


class TransferParametersHandler {
public:
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
};

#endif