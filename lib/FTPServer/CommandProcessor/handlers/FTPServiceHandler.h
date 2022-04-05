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
    static const int canHandleCmdsNumber; // can we bypass this somehow? :( // yes ;-))

    FTPDataProcessor* dataProcessor;

    void handleRetrCmd(CommandMessage*, Session*);
    void handleStorCmd(CommandMessage*, Session*);
    void handleStouCmd(CommandMessage*, Session*);
    void handleNoopCmd(CommandMessage*, Session*);
    void handlePwdCmd(CommandMessage*, Session*);
    void handleListCmd(CommandMessage*, Session*);
    void handleAborCmd(CommandMessage*, Session*);
    void handleMkdCmd(CommandMessage*, Session*);
    void handleSizeCmd(CommandMessage*, Session*);
    void handleDeleCmd(CommandMessage*, Session*);
    void handleRmdCmd(CommandMessage*, Session*);

    String getFilePath(Session*, String);
    String getUniqueFilePath(Session*, String);
    bool assertValidPathnameArgument(Session*, String);
    String getFileName(File);

public:
    FTPServiceHandler(FTPDataProcessor*);
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
    FTPDataProcessor* getFTPDataProcessor();
};

#endif