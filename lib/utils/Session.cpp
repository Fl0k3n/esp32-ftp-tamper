#include "Session.h"

Session::Session(WiFiClient* commandSock) :
    commandSocket(commandSock),
    transferState(commandSock->remoteIP().toString())
{
    init();
}

void Session::init() {
    sessionStatus = AWAIT_USERNAME;
    mode = ACTIVE; // active is default
    clearMessageBuff();
    workingDirectory = "/";
    isListenningForData = false;
}

Session::~Session() {
    if (commandSocket->connected())
        commandSocket->stop();
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

void Session::clearMessageBuff() {
    messageBuff = "";
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
