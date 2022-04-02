#include "FTPDataProcessor.h"

bool FTPDataProcessor::establishActiveSession(Session* session) {
    // 5.2.  CONNECTIONS
    // The server shall initiate the data connection from his own default data port (L-1) = 20
    // not sure if WiFiClient lets us choose local port

    TransferState* transferState = session->getTransferState();

    return transferState->dataSocket.connect(transferState->clientDataIP.c_str(), transferState->clientDataPort);
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


void FTPDataProcessor::handleDataTransfer(Session* session) {
    TransferState* transferState = session->getTransferState();

    if (!transferState->dataSocket.connected()) {
        if (transferState->status == READ_IN_PROGRESS) {
            transferState->cleanupTransfer();
            // peer closed connection when we still wanted to send data
            Serial.println("peer closed connection");
            return;
        }
        else {
            // peer finished writing TODO (this is expected state (((probably))))
        }
    }


    if (transferState->status == READ_IN_PROGRESS) {
        sendDataChunk(transferState);
    }
    else if (transferState->status == WRITE_IN_PROGRESS) {
        // TODO
    }
    else {
        // ???
    }

    if (transferState->status == FINISHED) {
        Serial.println("sent entire file");
        session->getCommandSocket()->print(ResponseMessage("226", "Closing data connection, file transfer successful").encode());
        transferState->cleanupTransfer();

        if (session->mode == PASSIVE) {
            session->getDataServerSocket()->stop();
        }
    }
}