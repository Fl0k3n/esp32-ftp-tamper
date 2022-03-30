#include "Session.h"

Session::Session(WiFiClient* commandSock) : commandSocket(commandSock) {
    // init session, working dir = /, etc...
    init();
}

void Session::init() {
    sessionStatus = AWAIT_USERNAME;
    transferStatus = NO_TRANSFER;
    clearMessageBuff();
    workingDirectory = "/";
}

Session::~Session() {
    if (dataSocket->connected())
        dataSocket->stop();
    if (commandSocket->connected())
        commandSocket->stop();
}

SessionStatus Session::getSessionStatus() {
    return sessionStatus;
}

void Session::setSessionStatus(SessionStatus newSessionStatus) {
    sessionStatus = newSessionStatus;
}

TransferStatus Session::getTransferStatus() {
    return transferStatus;
}

void Session::setTransferStatus(TransferStatus newTransferStatus) {
    transferStatus = newTransferStatus;
}

WiFiClient* Session::getCommandSocket() {
    return commandSocket;
}

WiFiClient* Session::getDataSocket() {
    return dataSocket;
}

void Session::setMessageBuff(String buff) {
    messageBuff = buff;
}

String Session::getMessageBuff() {
    return messageBuff;
}

void Session::clearMessageBuff() {
    messageBuff = "";
}

bool Session::waitingToLogOutAndNoTranfser() {
    return sessionStatus == LOGOUT && transferStatus == NO_TRANSFER;
}

void Session::setWorkingDirectory(String newWorkingDirectory) {
    workingDirectory = newWorkingDirectory;
}

String Session::getWorkingDirectory() {
    return workingDirectory;
}