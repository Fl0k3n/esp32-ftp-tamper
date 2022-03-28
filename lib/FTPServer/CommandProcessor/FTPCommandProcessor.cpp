#include "FTPCommandProcessor.h"

FTPCommandProcessor::FTPCommandProcessor(Session* session)
    : session(session) {}


void FTPCommandProcessor::listenForCommands() {
    WiFiClient* commandSocket = session->getCommandSocket();
    Serial.println("Listenning for commands...");

    while (true) {
        if (!commandSocket->connected()) {
            handleDisconnected();
        }
        else if (commandSocket->available()) {
            bool isMessageFullyRcvd = processSocketInput();

            if (isMessageFullyRcvd)
                handleMessage();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // i think it should be as low as possible, even 1? but also priority of this task should be very low so other tasks may work  
    }
}


void FTPCommandProcessor::handleDisconnected() {
    // TODO terminate rtos task, cleanup
    Serial.println("client disconnected");
}

// returns true if full command was received
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
    Serial.println("GOT");
    Serial.println(buff);

    session->setMessageBuff(buff);
    return entireMessageRcvd;
}

void FTPCommandProcessor::handleMessage() {
    Serial.println("IN HANDLE MESSAGE: ");
    String rawMessage = session->getMessageBuff();
    session->clearMessageBuff();

    Message msg = Message::buildFromString(rawMessage);
    msg.print();

    // if (msg.command == "...") else if...
}