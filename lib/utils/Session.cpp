#include "Session.h"

Session::Session(WiFiClient* commandSock) : commandSocket(commandSock) {
    // init session, working dir = /, etc...
}

Session::~Session() {
    // is it closed automatically when connection is lost?
    // commandSocket.stop();
}

SessionStatus Session::getStatus() {
    return sessionStatus;
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