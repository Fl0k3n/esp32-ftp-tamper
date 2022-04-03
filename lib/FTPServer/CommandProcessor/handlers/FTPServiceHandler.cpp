#include "FTPServiceHandler.h"


// no, we wont implement all
const String FTPServiceHandler::canHandleCmds[] = { "RETR", "STOR", "STOU", "APPE", "ALLO", "REST", "RNFR", "RNTO", "ABOR", "DELE", "RMD", "MKD", "PWD", "LIST", "NLST", "SITE", "SYST", "STAT", "HELP", "NOOP" };
const int FTPServiceHandler::canHandleCmdsNumber = sizeof(canHandleCmds) / sizeof(canHandleCmds[0]);


FTPServiceHandler::FTPServiceHandler(FTPDataProcessor* dataProcessor) :dataProcessor(dataProcessor) {

}

bool FTPServiceHandler::canHandle(CommandMessage* msg) {
    return canHandleCommand(msg->command, (String*)canHandleCmds, canHandleCmdsNumber);
}

void FTPServiceHandler::handleMessage(CommandMessage* msg, Session* session) {
    Serial.println("FTP service handler: handling ");
    msg->print();

    String cmd = msg->command;

    if (session->getSessionStatus() != AWAIT_COMMAND) {
        sendReply(session, "530", "Not logged in.");
        return;
    }

    if (cmd == "RETR") {
        handleRetrCmd(msg, session);
    }
    else if (cmd == "STOR") {
        handleStorCmd(msg, session);
    }
    else if (cmd == "STOU") {
        handleStouCmd(msg, session);
    }
    else if (cmd == "NOOP") {
        handleNoopCmd(msg, session);
    }
    else {
        Serial.println("not implemented");
        sendReply(session, "502", "Not implemented");
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

    TransferState* transferState = session->getTransferState();
    transferState->status = READ_IN_PROGRESS;
    transferState->openFile = requestedFile;
}

void FTPServiceHandler::handleStorCmd(CommandMessage* msg, Session* session) {
    if (!assertValidPathnameArgument(session, msg->data))
        return;

    String path;
    if (msg->command == "STOU") {
        // handle msg->command == STOU 
        path = getUniqueFilePath(session, msg->data);
    }
    else {
        path = getFilePath(session, msg->data);
    }

    Serial.print("Trying to open file in 'w' mode: ");
    Serial.println(path);

    File file = SD.open(path, "w");
    if (!file) {
        Serial.println("Failed to open file");
        sendReply(session, "553", "Failed to create file with a given path");
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

    TransferState* transferState = session->getTransferState();
    transferState->status = WRITE_IN_PROGRESS;
    transferState->openFile = file;
}

void FTPServiceHandler::handleStouCmd(CommandMessage* msg, Session* session) {
    handleStorCmd(msg, session);
}

void FTPServiceHandler::handleNoopCmd(CommandMessage* msg, Session* session) {
    sendReply(session, "200", "OK");
}

String FTPServiceHandler::getUniqueFilePath(Session* session, String relativePath) {
    String originalPath = getFilePath(session, relativePath);

    int index = 1;
    String resultPath = originalPath;
    while (SD.exists(resultPath)) {
        resultPath = originalPath;
        resultPath.concat(index++);
    }

    return resultPath;
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