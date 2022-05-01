#include "Session.h"

Session::Session(WiFiClient* commandSock, ChaCha* cipher) :
    commandSocket(commandSock),
    transferState(commandSock->remoteIP().toString(), cipher)
{
    init();
}

void Session::init() {
    sessionStatus = AWAIT_USERNAME;
    mode = ACTIVE; // active is default
    clearMessageBuff();
    workingDirectory = "/";
    isListenningForData = false;
    readLockCount = writeLockCount = 0;
    fileToRename = "";
}

SessionStatus Session::getSessionStatus() {
    return sessionStatus;
}

void Session::setSessionStatus(SessionStatus newSessionStatus) {
    sessionStatus = newSessionStatus;
}


WiFiClient* Session::getCommandSocket() {
    return commandSocket;
}

void Session::setMessageBuff(String buff) {
    messageBuff = buff;
}

String Session::getMessageBuff() {
    return messageBuff;
}

String Session::getFileToRename() {
    return fileToRename;
}

void Session::clearMessageBuff() {
    messageBuff = "";
}

void Session::clearFileToRename() {
    fileToRename = "";
}

void Session::setFileToRename(String file) {
    fileToRename = file;
}

bool Session::canBeLoggedOut() {
    return sessionStatus == LOGOUT && transferState.isDataConnectionClosed();
}

void Session::setWorkingDirectory(String newWorkingDirectory) {
    workingDirectory = newWorkingDirectory;
}

String Session::getWorkingDirectory() {
    return workingDirectory;
}

TransferState* Session::getTransferState() {
    return &transferState;
}

bool Session::shouldListenForDataConnections() {
    return isListenningForData;
}


void Session::startListenningForDataConnection(uint16_t port) {
    dataServerSocket = WiFiServer(port, 1); // max 1 client
    dataServerSocket.begin();
    dataPort = port;
    isListenningForData = true;
}

void Session::stopListenningForDataConnection() {
    isListenningForData = false;
}

WiFiServer* Session::getDataServerSocket() {
    return &dataServerSocket;
}


void Session::cleanupTransfer() {
    transferState.cleanupTransfer();

    if (mode == PASSIVE) {
        dataServerSocket.stop();
    }
}