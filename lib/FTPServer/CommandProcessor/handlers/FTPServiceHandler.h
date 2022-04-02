#ifndef FTP_SERVICE_HANDLER
#define FTP_SERVICE_HANDLER

#include <Arduino.h>

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "FTPCommandHandler.h"
#include "Session.h"
#include "DataProcessor/FTPDataProcessor.h"



class FTPServiceHandler : public FTPCommandHandler {
private:
    static const String canHandleCmds[];
    static const int canHandleCmdsNumber = 18; // can we bypass this somehow? :(

    FTPDataProcessor* dataProcessor;

    void handleRetrCmd(CommandMessage*, Session*);
    String getFilePath(Session*, String);
    bool assertValidPathnameArgument(Session*, String);

public:
    FTPServiceHandler(FTPDataProcessor*);
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
    FTPDataProcessor* getFTPDataProcessor();
};

#endif