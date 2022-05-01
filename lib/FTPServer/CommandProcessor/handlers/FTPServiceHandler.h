#ifndef FTP_SERVICE_HANDLER
#define FTP_SERVICE_HANDLER

#include <Arduino.h>

#include "CommandMessage.h"
#include "ResponseMessage.h"
#include "FTPCommandHandler.h"
#include "Session.h"
#include "DataProcessor/FTPDataProcessor.h"
#include "ftpconf.h"
#include "AccessControler.h"

class FTPServiceHandler : public FTPCommandHandler {
private:
    static const String canHandleCmds[];
    static const int canHandleCmdsNumber;

    FTPDataProcessor* dataProcessor;
    AccessControler* accessControler;

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
    void handleRnfrCmd(CommandMessage*, Session*);
    void handleRntoCmd(CommandMessage*, Session*);
    void handleAppeCmd(CommandMessage*, Session*);

    String getFilePath(Session*, String);
    String getUniqueFilePath(Session*, String);
    bool assertValidPathnameArgument(Session*, String);
    String getFileName(File);

    bool assertDataConnectionOpen(Session*);
    bool assertSDLock(Session*);
    bool assertCanWrite(Session*);
    void sendServiceUnavailableMsg(Session*);
    void finishedWriting(Session*);
public:
    FTPServiceHandler(FTPDataProcessor*, AccessControler*);
    bool canHandle(CommandMessage*);
    void handleMessage(CommandMessage*, Session*);
    FTPDataProcessor* getFTPDataProcessor();
};

#endif