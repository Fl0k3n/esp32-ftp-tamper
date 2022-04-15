#include "FTPDataProcessor.h"

FTPDataProcessor::FTPDataProcessor(ChaCha chacha, const uint8_t* cipherKey, const uint8_t* iv, const int cipherKeyLen, const int ivLen) {
    this->chacha = chacha;
    this->cipherKey = cipherKey;
    this->iv = iv;
    this->cipherKeyLen = cipherKeyLen;
    this->ivLen = ivLen;
}

bool FTPDataProcessor::prepareCipher() {
    chacha.clear();
    return chacha.setKey(cipherKey, cipherKeyLen) && chacha.setIV(iv, ivLen);
}

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

bool FTPDataProcessor::sendDataChunk(TransferState* transferState) {
    uint8_t rbuf[READ_BUFF_SIZE]; // this may be moved to a private class field

    size_t rd = transferState->openFile.read(rbuf, READ_BUFF_SIZE);

    if (rd > 0) {
        int written = transferState->dataSocket.write(rbuf, rd);
        // chacha.decrypt(rbuf, rbuf, rd);

        if (written != -1 && written < rd) {
            Serial.printf("not entire buff was written, wrote: %d, read: %d\n", written, rd);
            return false;
        }
    }
    else {
        transferState->status = FINISHED;
    }

    return true;
}

bool FTPDataProcessor::receiveDataChunk(TransferState* transferState) {
    uint8_t wbuf[WRITE_BUFF_SIZE];

    size_t rd = transferState->dataSocket.read(wbuf, WRITE_BUFF_SIZE);

    if (rd > 0) {
        // chacha.encrypt(wbuf, wbuf, rd);
        int position = transferState->openFile.position();
        int written = transferState->openFile.write(wbuf, rd);

        int tries = 1;
        while (written < rd && tries <= 3) {
            transferState->openFile.seek(position);
            written = transferState->openFile.write(wbuf, rd);
            tries++;
        }

        if (written < rd) {
            return false;
        }

        // Serial.println(wbuf);
    }
    return true;
}

void FTPDataProcessor::handleFailedTransfer(Session* session) {
    session->getCommandSocket()->print(
        ResponseMessage("451", "Requested action aborted: error in processing received data; Closing data connection").encode());
    session->cleanupTransfer();
}


void FTPDataProcessor::handleDataTransfer(Session* session) {
    TransferState* transferState = session->getTransferState();

    if (!transferState->dataSocket.connected()) {
        if (transferState->status == READ_IN_PROGRESS) {
            // peer closed connection when we still wanted to send data
            session->cleanupTransfer();
            Serial.println("WARN: peer closed connection");
            return;
        }
        else if (transferState->status == WRITE_IN_PROGRESS) {
            transferState->status = FINISHED;
        }
    }


    if (transferState->status == READ_IN_PROGRESS) {
        bool success = sendDataChunk(transferState);
        if (!success) {
            Serial.println("ERROR: Failed to send data chunk");
            handleFailedTransfer(session);
        }

    }
    else if (transferState->status == WRITE_IN_PROGRESS) {
        bool success = receiveDataChunk(transferState);

        if (!success) {
            Serial.println("ERROR: Failed to receive data chunk");
            handleFailedTransfer(session);
        }
    }

    if (transferState->status == FINISHED) {
        Serial.println();
        Serial.println("operation on file finished successfully");
        session->getCommandSocket()->print(ResponseMessage("226", "Closing data connection, file transfer successful").encode());
        Serial.print("Is still connected? ");
        Serial.println(session->getCommandSocket()->connected());
        session->cleanupTransfer();
    }
}