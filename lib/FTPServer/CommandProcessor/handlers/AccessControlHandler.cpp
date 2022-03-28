#include "AccessControlHandler.h"


AccessControlHandler::AccessControlHandler(String username, String password)
    : username(username), password(password) {}

bool AccessControlHandler::canHandle(CommandMessage* msg) {
    String cmd = msg->command;
    return cmd == "USER" || cmd == "PASS" || cmd == "ACCT" ||
        cmd == "CWD" || cmd == "CDUP" || cmd == "SMMT" || cmd == "REIN" || cmd == "QUIT";
}


void AccessControlHandler::handleMessage(CommandMessage* msg, Session* session) {
    Serial.println("Access control handler: handling ");
    msg->print();


    if (msg->command == "USER") {
        if (session->getStatus() == AWAIT_USERNAME) {
            Serial.println("got username as expected");
            // todo validate
            ResponseMessage response("331", "User name ok, need password");

            session->getCommandSocket()->print(response.encode());
            Serial.println("sent response");
        }
        else {
            // ??
        }
    }
    else if (msg->command == "PASS") {
        //...
    }
}