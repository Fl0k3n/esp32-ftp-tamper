#include "FTPServiceHandler.h"


// no, we wont implement all
const String FTPServiceHandler::canHandleCmds[] = { "RETR", "STOR", "STOU", "APPE", "ALLO", "REST", "RNFR", "RNTO", "ABOR", "DELE", "RMD", "MKD", "PWD", "LIST", "NLST", "SITE", "SYST", "STAT", "HELP", "NOOP", "SIZE" };
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
    else if (cmd == "PWD") {
        handlePwdCmd(msg, session);
    }
    else if (cmd == "LIST") {
        handleListCmd(msg, session);
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

void FTPServiceHandler::handlePwdCmd(CommandMessage* msg, Session* session) {
    sendReply(session, "257", "\"" + session->getWorkingDirectory() + "\" is current working directory");
}

void FTPServiceHandler::handleListCmd(CommandMessage* msg, Session* session) {
    if (session->getTransferState()->isDataConnectionClosed()) {
        sendReply(session, "425", "Can't open data connection.");
    }

    WiFiClient* dataSocket = session->getTransferState()->getDataSocket();

    sendReply(session, "150", "Accepted data connection");
    uint16_t files = 0;
    File dir = SD.open(session->getWorkingDirectory());
    if ((!dir) || (!dir.isDirectory())) {
        sendReply(session, "550", "Can't open this directory " + session->getWorkingDirectory());
    }
    else {
        File file = dir.openNextFile();
        while (file) {
            String fileName, fileSize;
            fileName = file.name();
            int sep = fileName.lastIndexOf('/');
            fileName = fileName.substring(sep + 1);
            fileSize = String(file.size());
            if (file.isDirectory()) {
                dataSocket->println("01-01-2000  00:00AM <DIR> " + fileName);
            }
            else {
                dataSocket->println("01-01-2000  00:00AM " + fileSize + " " + fileName);
            }
            files++;
            file = dir.openNextFile();
        }
        sendReply(session, "226", String(files) + " matches total");
    }
    dataSocket->stop();
}

String FTPServiceHandler::getUniqueFilePath(Session* session, String relativePath) {
    String originalPath = getFilePath(session, relativePath);

    String resultPath = originalPath;
    int extensionPartIndex = originalPath.indexOf('.');
    if (extensionPartIndex == -1) {
        int fileIndex = 1;
        while (SD.exists(resultPath)) {
            resultPath = originalPath;
            resultPath.concat(fileIndex++);
        }
    }
    else {
        String extensionPart = originalPath.substring(extensionPartIndex);
        String pathWithoutExtension = originalPath.substring(0, extensionPartIndex);

        int fileIndex = 1;
        while (SD.exists(resultPath)) {
            resultPath = pathWithoutExtension;
            resultPath.concat(fileIndex++);
            resultPath.concat(extensionPart);
        }
    }

    Serial.println(resultPath);

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