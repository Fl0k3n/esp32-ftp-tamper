#include "FTPCommandProcessor.h"

FTPCommandProcessor::FTPCommandProcessor(Session* session, AccessControlHandler* commandHandler,
    FTPServiceHandler* ftpServiceHandler, TransferParametersHandler* transferParametersHandler)
    : session(session), dataProcessor(ftpServiceHandler->getFTPDataProcessor()), accessControlHandler(commandHandler),
    ftpServiceHandler(ftpServiceHandler), transferParametersHandler(transferParametersHandler) {}


void FTPCommandProcessor::listenForCommands() {
    WiFiClient* commandSocket = session->getCommandSocket();

    commandSocket->print(ResponseMessage("220", "Service ready").encode());
    Serial.println("Listenning for commands...");

    session->setSessionStatus(AWAIT_USERNAME);

    while (true) {
        Serial.print("beep\t"); //debug
        if (!commandSocket->connected() || session->canBeLoggedOut()) {
            handleDisconnected();
            return;
        }

        if (session->getSessionStatus() == REINITIALIZATION) {
            if (session->getTransferState()->isDataConnectionClosed()) {
                session->init();
                commandSocket->print(ResponseMessage("220", "Service ready").encode());
                Serial.println("Reinitialization completed. Listenning for commands...");
            }
        }

        if (commandSocket->available()) {
            bool isMessageFullyRcvd = processSocketInput();

            if (isMessageFullyRcvd)
                handleMessage();
            else
                Serial.println("message not fully received");
        }

        if (session->shouldListenForDataConnections()) {
            Serial.print("should\t");
            checkDataConnections();
        }

        // not sure if this should be here
        if (session->getTransferState()->isTransferInProgress() && !session->shouldListenForDataConnections()) {
            Serial.print("peeb\t"); //debug
            dataProcessor->handleDataTransfer(session);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // i think it should be as low as possible, even 1? but also priority of this task should be very low so other tasks may work  
    }
}

void FTPCommandProcessor::checkDataConnections() {
    WiFiServer* dataServerSocket = session->getDataServerSocket();

    if (dataServerSocket->hasClient()) {
        Serial.println("Got data socket");
        WiFiClient dataClientSocket = dataServerSocket->available();
        session->stopListenningForDataConnection();

        TransferState* transferState = session->getTransferState();
        transferState->dataSocket = dataClientSocket;

        // session->getCommandSocket()->print(ResponseMessage("225", "Data connection open; no transfer in progress.").encode());
    }
    // else if (!session->getTransferState()->isTransferInProgress()) {
    //     // TODO timeout?
    //     Serial.println("no data connection yet....");
    // }
    else {
        Serial.println("no data connection yet...");
    }
}


void FTPCommandProcessor::handleDisconnected() {
    ResponseMessage response("221", "Disconnected from the server");
    session->getCommandSocket()->print(response.encode());
    Serial.println("client disconnected");
    session->getDataServerSocket()->close();
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