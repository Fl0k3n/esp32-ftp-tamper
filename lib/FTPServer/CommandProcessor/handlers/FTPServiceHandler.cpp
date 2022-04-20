#include "FTPServiceHandler.h"


// no, we wont implement all
const String FTPServiceHandler::canHandleCmds[] = { "RETR", "STOR", "STOU", "APPE", "ALLO", "REST", "RNFR", "RNTO", "MDTM", "ABOR", "DELE", "RMD", "MKD", "PWD", "LIST", "NLST", "SITE", "SYST", "STAT", "HELP", "NOOP", "SIZE" };
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
    else if (cmd == "ABOR") {
        handleAborCmd(msg, session);
    }
    else if (cmd == "MKD") {
        handleMkdCmd(msg, session);
    }
    else if (cmd == "SIZE") {
        handleSizeCmd(msg, session);
    }
    else if (cmd == "RMD") {
        handleRmdCmd(msg, session);
    }
    else if (cmd == "DELE") {
        handleDeleCmd(msg, session);
    }
    else if (cmd == "RNFR") {
        handleRnfrCmd(msg, session);
    }
    else if (cmd == "RNTO") {
        handleRntoCmd(msg, session);
    }
    else if (cmd == "APPE") {
        handleAppeCmd(msg, session);
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

    File requestedFile = SD.open(path, FILE_READ);
    if (!requestedFile) {
        Serial.println("Failed to open file");
        sendReply(session, "550", "File not found.");
        return;
    }

    sendReply(session, "150", "File status okay; about to open data connection.");

    if (assertDataConnectionOpen(session)) {
        TransferState* transferState = session->getTransferState();
        transferState->status = READ_IN_PROGRESS;
        transferState->openFile = requestedFile;
        transferState->openFilePath = path;
        dataProcessor->prepareCipher();
    }
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

    File file;
    if (msg->command == "APPE") {
        file = SD.open(path, FILE_APPEND);
    } else {
        file = SD.open(path, FILE_WRITE);
    }
    if (!file) {
        Serial.println("Failed to open file");
        sendReply(session, "553", "Failed to create file with a given path");
        return;
    }

    sendReply(session, "150", "File status okay; about to open data connection.");

    if (assertDataConnectionOpen(session)) {
        TransferState* transferState = session->getTransferState();
        transferState->status = WRITE_IN_PROGRESS;
        transferState->openFile = file;
        transferState->openFilePath = path;
        if (msg->command != "APPE") {
            dataProcessor->prepareCipher();
        }
    }
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
    if (!assertDataConnectionOpen(session))
        return;

    WiFiClient* dataSocket = session->getTransferState()->getDataSocket();

    sendReply(session, "150", "Accepted data connection");

    String path;
    if (msg->data == "") {
        path = session->getWorkingDirectory();
    }
    else {
        path = getFilePath(session, msg->data);
    }

    File dir = SD.open(session->getWorkingDirectory(), FILE_READ);
    if ((!dir)) {
        sendReply(session, "550", "Can't open this file/directory " + path);
    }
    else {
        if (dir.isDirectory()) {
            Serial.println("listing directory");
            uint16_t files = 0;
            File file = dir.openNextFile();

            while (file) {
                String fileName = getFileName(file);
                String fileSize = String(file.size());

                if (file.isDirectory()) {
                    dataSocket->println("01-01-1970  00:00AM <DIR> " + fileName);
                }
                else {
                    dataSocket->println("01-01-1970  00:00AM " + fileSize + " " + fileName);
                }

                files++;
                file = dir.openNextFile();
            }
            sendReply(session, "226", String(files) + " matches total");
        }
        else {
            Serial.println("listing file");
            String fileName = getFileName(dir);
            String fileSize = String(dir.size());
            dataSocket->println("01-01-1970  00:00AM " + fileSize + " " + fileName);
            sendReply(session, "226", "Requested file action successful. Closing data connection");
        }
    }

    dataSocket->stop();
    if (session->mode == PASSIVE) {
        session->getDataServerSocket()->stop();
    }
}

void FTPServiceHandler::handleAborCmd(CommandMessage* msg, Session* session) {
    Serial.println("Handling abort...");
    session->stopListenningForDataConnection();

    if (session->getTransferState()->isTransferInProgress())
        sendReply(session, "426", "Transfer aborted");

    session->cleanupTransfer();

    sendReply(session, "226", "Aborting OK.");
}

void FTPServiceHandler::handleMkdCmd(CommandMessage* msg, Session* session) {
    if (msg->data == "") {
        sendReply(session, "501", "No directory name given");
        return;
    }

    String dirPath = getFilePath(session, msg->data);

    if (SD.exists(dirPath)) {
        sendReply(session, "550", "Directory with this path already exists.");
        return;
    }

    if (SD.mkdir(dirPath)) {
        sendReply(session, "257", "\"" + dirPath + "\" created successfully");
        Serial.println("Created " + dirPath);
    }
    else {
        sendReply(session, "550", "Unable to create directory \"" + dirPath + "\"");
        Serial.println("Unable to create " + dirPath);
    }
}

void FTPServiceHandler::handleSizeCmd(CommandMessage* msg, Session* session) {
    if (!assertValidPathnameArgument(session, msg->data))
        return;


    String filePath = getFilePath(session, msg->data);

    if (!SD.exists(filePath)) {
        sendReply(session, "550", "File with given name doesn't exist.");
        return;
    }

    File file = SD.open(filePath, FILE_READ);

    if (file) {
        sendReply(session, "213", "File size: " + String(file.size()));
    }
    else {
        sendReply(session, "450", "Wasn't able to open " + filePath);
    }
}

void FTPServiceHandler::handleDeleCmd(CommandMessage* msg, Session* session) {
    if (!assertValidPathnameArgument(session, msg->data))
        return;


    String filePath = getFilePath(session, msg->data);

    if (!SD.exists(filePath)) {
        sendReply(session, "550", "File/directory with this path does not exist");
        return;
    }

    bool res;
    if (msg->command == "DELE")
        res = SD.remove(filePath);
    else
        res = SD.rmdir(filePath);

    if (res)
        sendReply(session, "250", "Deleted successfully.");
    else {
        if (msg->command == "DELE")
            sendReply(session, "450", "Couldn't delete file " + filePath);
        else
            sendReply(session, "550", "Couldn't delete directory " + filePath);
    }
}

void FTPServiceHandler::handleRmdCmd(CommandMessage* msg, Session* session) {
    handleDeleCmd(msg, session);
}

void FTPServiceHandler::handleRnfrCmd(CommandMessage* msg, Session* session) {
    if (!assertValidPathnameArgument(session, msg->data))
        return;


    String fileToRename = getFilePath(session, msg->data);

    if (!SD.exists(fileToRename)) {
        sendReply(session, "550", "File with such name was not found");
        return;
    }

    session->setFileToRename(fileToRename);

    sendReply(session, "350", "RNFR accepted. Waiting for RNTO command.");
}

void FTPServiceHandler::handleRntoCmd(CommandMessage* msg, Session* session) {
    if (!assertValidPathnameArgument(session, msg->data))
        return;


    if (session->getFileToRename() == "") {
        sendReply(session, "503", "Send filename of the file to rename first");
        return;
    }

    String newFileName = getFilePath(session, msg->data);

    if (SD.exists(newFileName)) {
        sendReply(session, "553", "File with given filename already exists");
        return;
    }

    if (SD.rename(session->getFileToRename(), newFileName)) {
        Serial.println("Renaming " + session->getFileToRename() + " to " + newFileName + " finished successfully");
        sendReply(session, "250", "File renamed successfully");
    } else {
        Serial.println("Renaming " + session->getFileToRename() + " to " + newFileName + " failed");
        sendReply(session, "451", "Renaming file failed");
    }
    session->clearFileToRename();
}

void FTPServiceHandler::handleAppeCmd(CommandMessage* msg, Session* session) {
    handleStorCmd(msg, session);
}

String FTPServiceHandler::getFileName(File file) {
    String fileName = file.name();
    int sep = fileName.lastIndexOf('/');
    return fileName.substring(sep + 1);
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



bool FTPServiceHandler::assertDataConnectionOpen(Session* session) {
    Serial.print("Awaiting data connection");
    bool connectionEstablished = false;

    // TODO what if its already open?
    if (session->getTransferState()->getDataSocket()->connected()) {
        Serial.println("CRITICAL: opening another data connection");
        return true;
    }

    for (int i = 0; i < DATA_CONNECTION_TIMEOUT_MILLIS / 10; i++) {
        if (!session->getCommandSocket()->connected()) {
            Serial.println("client disconnected while waiting for data connection");
            return false;
        }

        if (dataProcessor->establishDataConnection(session)) {
            connectionEstablished = true;
            break;
        }

        Serial.print(".");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    Serial.println();

    if (connectionEstablished) {
        Serial.print("Data connection established in ");
    }
    else {
        Serial.print("Failed to establish connection in ");
    }

    Serial.printf("%s mode\n", session->mode == ACTIVE ? "active" : "passive");

    if (!connectionEstablished) {
        sendReply(session, "425", "No data connection.");
        // probably not needed
        session->getTransferState()->getDataSocket()->stop();
    }

    return connectionEstablished;
}