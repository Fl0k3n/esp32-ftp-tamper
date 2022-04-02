#include "TransferParametersHandler.h"

const String TransferParametersHandler::canHandleCmds[] = { "PORT", "PASV", "TYPE", "STRU", "MODE" };


bool TransferParametersHandler::canHandle(CommandMessage* msg) {
    return canHandleCommand(msg->command, (String*)canHandleCmds, canHandleCmdsNumber);
}

void TransferParametersHandler::handleMessage(CommandMessage* msg, Session* session) {
    Serial.println("Transfer parameters handler: handling ");
    msg->print();
    String cmd = msg->command;

    if (cmd == "PORT") {
        handlePortCmd(msg, session);
    }
    else if (cmd == "PASV") {
        handlePasvCmd(msg, session);
    }
    else {
        Serial.println("not implemented");
    }
}


void TransferParametersHandler::handlePortCmd(CommandMessage* msg, Session* session) {
    // PORT h1,h2,h3,h4,p1,p2
    uint8_t ipOctets[4];
    uint8_t portOctets[2];

    int successfullyParsed = sscanf(msg->data.c_str(), "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu",
        &ipOctets[0], &ipOctets[1], &ipOctets[2], &ipOctets[3], &portOctets[0], &portOctets[1]);

    if (successfullyParsed != 6 || msg->data.length() > 24) {
        Serial.print("bad addres-port combination: ");
        Serial.println(msg->data);
        sendReply(session, "500", "Invalid IP address-port combination");
        return;
    }

    IPAddress ip(ipOctets);
    uint16_t port = 256 * portOctets[0] + portOctets[1];

    session->getTransferState()->clientDataIP = ip.toString();
    session->getTransferState()->clientDataPort = port;

    Serial.print("Changed address to ");
    Serial.print(ip.toString());
    Serial.printf(":%hu\n", port);

    sendReply(session, "200", "Command okay.");
}


void TransferParametersHandler::handlePasvCmd(CommandMessage* msg, Session* session) {
    if (session->mode == PASSIVE) {
        // TODO handle already PASSIVE
        Serial.println("already in passive");
        return;
    }

    session->mode = PASSIVE;

    // TODO, server has to choose the port, hardcoded for now
    uint16_t port = 20;
    IPAddress ip = WiFi.localIP();

    session->startListenningForDataConnection(port);

    String ipStr = ip.toString();
    ipStr.replace(".", ",");

    char buff[24];
    // h1,h2,h3,h4,p1,p2
    sprintf(buff, "%s,%hhu,%hhu", ipStr.c_str(), port / 256, port % 256);

    sendReply(session, "227", String(buff));
    Serial.printf("sent pasv response: ->%s<-\n", buff);
}