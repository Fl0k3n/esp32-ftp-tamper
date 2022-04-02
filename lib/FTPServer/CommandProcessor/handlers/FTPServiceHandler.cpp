#include "FTPServiceHandler.h"


// no, we wont implement all
const String FTPServiceHandler::canHandleCmds[] = { "RETR", "STOR", "STOU", "APPE", "ALLO", "REST", "RNFR", "RNTO", "ABOR", "DELE", "RMD", "MKD", "PWD", "LIST", "NLST", "SITE", "SYST", "STAT" };


FTPServiceHandler::FTPServiceHandler(FTPDataProcessor* dataProcessor) :dataProcessor(dataProcessor) {

}

bool FTPServiceHandler::canHandle(CommandMessage* msg) {
    return canHandleCommand(msg->command, (String*)canHandleCmds, canHandleCmdsNumber);
}

void FTPServiceHandler::handleMessage(CommandMessage* msg, Session* session) {
    Serial.println("FTP service handler: handling ");
    msg->print();

    String cmd = msg->command;

    if (cmd == "RETR") {
        handleRetrCmd(msg, session);
    }
}


void FTPServiceHandler::handleRetrCmd(CommandMessage* msg, Session* session) {
    if (!assertValidPathnameArgument(session, msg->data))
        return;

    String path = getFilePath(session, msg->data);
    Serial.print("Trying to open file in 'r' mode: ");
    Serial.println(path);

    File requestedFile = SD.open(path, "r");
    if (!requestedFile) {
        Serial.println("Failed to open file");
        sendReply(session, "550", "File not found.");
        return;
    }

    sendReply(session, "150", "File status okay; about to open data connection.");


    if (session->mode == ACTIVE) {
        bool sessionEstb = dataProcessor->establishActiveSession(session);
        if (!sessionEstb) {
            Serial.println("Failed to establish session");
            sendReply(session, "425", "Can't open data connection.");
            return;
        }

    }
    else {
        // Passive 
        // TODO, assert that data connection is open (from client to server)
    }

    TransferState* transferState = session->getTransferState();
    transferState->status = READ_IN_PROGRESS;
    transferState->openFile = requestedFile;
}



String FTPServiceHandler::getFilePath(Session* session, String relativePath) {
    if (relativePath.startsWith("/"))
        return relativePath;

    String cwd = session->getWorkingDirectory();

    return cwd == "/" ? cwd + relativePath : cwd + "/" + relativePath;
}

bool FTPServiceHandler::assertValidPathnameArgument(Session* session, String pathname) {
    if (pathname == "") {
        sendReply(session, "501", "Expected filename.");
        return false;
    }

    // not sure if thats all
    return true;

}


FTPDataProcessor* FTPServiceHandler::getFTPDataProcessor() {
    return dataProcessor;
}