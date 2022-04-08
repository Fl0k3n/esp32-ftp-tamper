#include "FTPDataProcessor.h"

bool FTPDataProcessor::establishDataConnection(Session* session) {
    TransferState* transferState = session->getTransferState();

    if (session->mode == PASSIVE) {
        WiFiServer* dataServerSocket = session->getDataServerSocket();
        if (dataServerSocket->hasClient()) {
            WiFiClient dataClientSocket = dataServerSocket->available();
            session->stopListenningForDataConnection();

            transferState->dataSocket = dataClientSocket;
            return true;
        }

        return false;
    }

    // 5.2.  CONNECTIONS
    // The server shall initiate the data connection from his own default data port (L-1) = 20
    // not sure if WiFiClient lets us choose local port
    return transferState->dataSocket
        .connect(transferState->clientDataIP.c_str(), transferState->clientDataPort);
}

void FTPDataProcessor::sendDataChunk(TransferState* transferState) {
    char rbuf[READ_BUFF_SIZE]; // this may be moved to a private class field

    int rd = transferState->openFile.readBytes(rbuf, READ_BUFF_SIZE);
    if (rd > 0) {
        int written = transferState->dataSocket.write(rbuf, rd);

        if (written != -1 && written < rd) {
            Serial.printf("not entire buff was written, wrote: %d, read: %d\n", written, rd);
            // TODO
        }
    }
    else {
        transferState->status = FINISHED;
    }
}

bool FTPDataProcessor::receiveDataChunk(TransferState* transferState) {
    char wbuf[WRITE_BUFF_SIZE];

    int rd = transferState->dataSocket.readBytes(wbuf, WRITE_BUFF_SIZE);
    if (rd > 0) {
        int position = transferState->openFile.position();
        int written = transferState->openFile.write((uint8_t*)wbuf, rd);

        // not sure about this one, maybe immediately return 451 instead?
        int tries = 1;
        while (written < rd && tries <= 3) {
            transferState->openFile.seek(position);
            written = transferState->openFile.write((uint8_t*)wbuf, rd);
            tries++;
        }

        if (written < rd) {
            return false;
        }

        Serial.println(wbuf);
    }
    return true;
}


void FTPDataProcessor::handleDataTransfer(Session* session) {
    TransferState* transferState = session->getTransferState();

    if (!transferState->dataSocket.connected()) {
        if (transferState->status == READ_IN_PROGRESS) {
            transferState->cleanupTransfer();

            if (session->mode == PASSIVE) {
                session->getDataServerSocket()->stop();
            }
            // peer closed connection when we still wanted to send data
            Serial.println("peer closed connection");
            return;
        }
        else if (transferState->status == WRITE_IN_PROGRESS) {
            // peer finished writing TODO (this is expected state (((probably))))
            transferState->status = FINISHED;
        }
    }


    if (transferState->status == READ_IN_PROGRESS) {
        sendDataChunk(transferState);
    }
    else if (transferState->status == WRITE_IN_PROGRESS) {
        bool success = receiveDataChunk(transferState);

        if (!success) {
            session->getCommandSocket()->print(ResponseMessage("451", "Requested action aborted: error in processing received data; Closing data connection").encode());
            transferState->cleanupTransfer();

            if (session->mode == PASSIVE) {
                session->getDataServerSocket()->stop();
            }
        }
    }

    if (transferState->status == FINISHED) {
        Serial.println("operation on file finished successfully");
        session->getCommandSocket()->print(ResponseMessage("226", "Closing data connection, file transfer successful").encode());
        transferState->cleanupTransfer();

        if (session->mode == PASSIVE) {
            session->getDataServerSocket()->stop();
        }
    }
}