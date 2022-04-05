#include "TransferParametersHandler.h"

const String TransferParametersHandler::canHandleCmds[] = { "PORT", "PASV", "TYPE", "STRU", "MODE" };
const int TransferParametersHandler::canHandleCmdsNumber = sizeof(canHandleCmds) / sizeof(canHandleCmds[0]);

TransferParametersHandler::TransferParametersHandler() {
    // TODO, server has to choose the port, hardcoded for now
    dataPort = 50009; // must be unprivilieged >1024
    IPAddress ip = WiFi.localIP();

    ipStringForPasv = ip.toString();
    ipStringForPasv.replace(".", ",");

    // h1,h2,h3,h4,p1,p2
    sprintf(pasvResponse, "%s,%hhu,%hhu", ipStringForPasv.c_str(), dataPort / 256, dataPort % 256);
}

bool TransferParametersHandler::canHandle(CommandMessage* msg) {
    return canHandleCommand(msg->command, (String*)canHandleCmds, canHandleCmdsNumber);
}

void TransferParametersHandler::handleMessage(CommandMessage* msg, Session* session) {
    Serial.println("Transfer parameters handler: handling ");
    msg->print();
    String cmd = msg->command;

    if (session->getSessionStatus() != AWAIT_COMMAND) {
        sendReply(session, "530", "Not logged in.");
        return;
    }

    if (cmd == "PORT") {
        handlePortCmd(msg, session);
    }
    else if (cmd == "PASV") {
        handlePasvCmd(msg, session);
    }
    else if (cmd == "TYPE") {
        handleTypeCmd(msg, session);
    }
    else if (cmd == "STRU") {
        handleStruCmd(msg, session);
    }
    else if (cmd == "MODE") {
        handleModeCmd(msg, session);
    }
    else {
        Serial.println("not implemented");
        sendReply(session, "502", "Not implemented");
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
    if (session->getTransferState()->isTransferInProgress()) {
        // TODO review this section, I guess it shouldnt handle it like that if there is a pasv from a client while transfer in progress
        Serial.println("already in passive");
        sendPasvResponse(session);
        return;
    }

    session->mode = PASSIVE;
    session->startListenningForDataConnection(dataPort);
    sendPasvResponse(session);
}

void TransferParametersHandler::sendPasvResponse(Session* session) {
    sendReply(session, "227", String(pasvResponse));
    Serial.printf("sent pasv response: ->%s<-\n", pasvResponse);
}

void TransferParametersHandler::handleTypeCmd(CommandMessage* msg, Session* session) {
    if (msg->data == "A" || msg->data == "A N")
        sendReply(session, "200", "ASCII Non-print type");
    else if (msg->data == "A T" || msg->data == "A C" || msg->data.startsWith("E") || msg->data.startsWith("I") || msg->data.startsWith("L"))
        sendReply(session, "504", "Only ASCII Non-print type is supported.");
    else
        sendReply(session, "501", "Invalid type");
}

void TransferParametersHandler::handleStruCmd(CommandMessage* msg, Session* session) {
    if (msg->data == "F")
        sendReply(session, "200", "File");
    else if (msg->data == "R")
        sendReply(session, "504", "Record structure not available"); // should be implemented and changed to 200
    else if (msg->data == "P")
        sendReply(session, "504", "Page structure not available");
    else
        sendReply(session, "501", "Invalid file structure");
}

void TransferParametersHandler::handleModeCmd(CommandMessage* msg, Session* session) {
    if (msg->data == "S")
        sendReply(session, "200", "Stream mode");
    else if (msg->data == "B" || msg->data == "C")
        sendReply(session, "504", "Only stream transfer mode is available");
    else
        sendReply(session, "501", "Invalid transfer mode");
}