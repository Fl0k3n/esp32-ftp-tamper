#include "FTPCommandProcessor.h"

FTPCommandProcessor::FTPCommandProcessor(Session* session, AccessControlHandler* commandHandler,
    FTPServiceHandler* ftpServiceHandler, TransferParametersHandler* transferParametersHandler)
    : session(session), accessControlHandler(commandHandler),
    ftpServiceHandler(ftpServiceHandler), transferParametersHandler(transferParametersHandler) {}


void FTPCommandProcessor::listenForCommands() {
    WiFiClient* commandSocket = session->getCommandSocket();

    commandSocket->print(ResponseMessage("220", "Service ready").encode());
    Serial.println("Listenning for commands...");

    session->setSessionStatus(AWAIT_USERNAME);

    while (true) {
        if (!commandSocket->connected() || session->waitingToLogOutAndNoTranfser()) {
            handleDisconnected();
            return;
        }

        if (session->getSessionStatus() == REINITIALIZATION) {
            if (session->getTransferStatus() == NO_TRANSFER) {
                session->init();
                commandSocket->print(ResponseMessage("220", "Service ready").encode());
                Serial.println("Reinitialization completed. Listenning for commands...");
            }
        }

        if (commandSocket->available()) {
            bool isMessageFullyRcvd = processSocketInput();

            if (isMessageFullyRcvd)
                handleMessage();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // i think it should be as low as possible, even 1? but also priority of this task should be very low so other tasks may work  
    }
}


void FTPCommandProcessor::handleDisconnected() {
    ResponseMessage response("221", "Disconnected from the server");
    session->getCommandSocket()->print(response.encode());
    Serial.println("client disconnected");
}

// returns true if full command was received
// 5.3 section
bool FTPCommandProcessor::processSocketInput() {
    WiFiClient* socket = session->getCommandSocket();

    String buff = session->getMessageBuff();
    bool CR_charFound = buff.length() > 0 && buff[buff.length() - 1] == '\r';

    bool entireMessageRcvd = false;
    char c;

    while ((c = socket->read()) != -1) {
        buff += c;

        if (c == '\n' && CR_charFound) {
            entireMessageRcvd = true;
            break;
        }

        if (c == '\r')
            CR_charFound = true;
        else {
            // reset \r if it was in the middle of a message
            CR_charFound = false;
        }
    }

    session->setMessageBuff(buff);
    return entireMessageRcvd;
}

void FTPCommandProcessor::handleMessage() {
    Serial.println("IN HANDLE MESSAGE: ");
    String rawMessage = session->getMessageBuff();
    session->clearMessageBuff();

    CommandMessage msg = CommandMessage::decode(rawMessage);
    msg.print();

    // TODO if we won't need to pass more specific params we can make a single interface for these handlers
    if (accessControlHandler->canHandle(&msg)) {
        accessControlHandler->handleMessage(&msg, session);
    }
    else if (ftpServiceHandler->canHandle(&msg)) {
        ftpServiceHandler->handleMessage(&msg, session);
    }
    else if (transferParametersHandler->canHandle(&msg)) {
        transferParametersHandler->handleMessage(&msg, session);
    }
    else {
        Serial.println("got unexpected message");
        msg.print();
    }
}