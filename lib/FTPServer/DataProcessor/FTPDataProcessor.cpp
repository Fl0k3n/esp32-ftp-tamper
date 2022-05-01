#include "FTPDataProcessor.h"

FTPDataProcessor::FTPDataProcessor(const uint8_t* cipherKey, AccessControler* accessControler)
    : cipherKey(cipherKey), accessControler(accessControler) {
}

bool FTPDataProcessor::assertValidCipherConfig() {
    ChaCha chacha = ChaCha(CHACHA_ROUNDS);

    if (!chacha.setKey(cipherKey, KEY_LEN)) {
        Serial.println("Key length is not supported or the key is weak and unusable by this cipher.");
        return false;
    }

    uint8_t ivPlaceholder[IV_LEN];

    if (!chacha.setIV(ivPlaceholder, IV_LEN)) {
        Serial.println("IV length is not supported.");
        return false;
    }

    return true;
}

// takes file as argument to assert that its already open
bool FTPDataProcessor::prepareCipher(Session* session, String fileMode, File* openFile) {
    TransferState* transferState = session->getTransferState();
    ChaCha* chacha = transferState->cipher;

    chacha->clear();
    accessControler->tryLockSd();
    bool success = true;

    if (fileMode == FILE_WRITE) {
        generateIV(transferState->iv);
        if (openFile->write(transferState->iv, IV_LEN) < IV_LEN) {
            Serial.print("CRITICAL: Failed to write IV to ");
            Serial.println(openFile->name());
            success = false;
        }
    }
    else if (fileMode == FILE_READ) {
        if (openFile->read(transferState->iv, IV_LEN) < IV_LEN) {
            Serial.print("CRITICAL: Failed to read IV from ");
            Serial.println(openFile->name());
            success = false;
        }
    }
    else {
        Serial.print("CRITICAL: Requested cipher init for file mode: ");
        Serial.println(fileMode);
        success = false;
    }

    accessControler->unlockSD();

    return success && chacha->setKey(cipherKey, KEY_LEN) && chacha->setIV(transferState->iv, IV_LEN);
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
    accessControler->tryLockSd(); // TODO
    int rd = transferState->openFile.readBytes(transferState->buf, FTP_BUF_SIZE);
    accessControler->unlockSD();
    uint8_t* buf = (uint8_t*)transferState->buf;

    if (rd > 0) {
        transferState->cipher->decrypt(buf, buf, rd);
        int written = transferState->dataSocket.write(buf, rd);

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
    uint8_t* buf = (uint8_t*)transferState->buf;
    int rd = transferState->dataSocket.readBytes(buf, FTP_BUF_SIZE);

    if (rd > 0) {
        transferState->cipher->encrypt(buf, buf, rd);
        int position = transferState->openFile.position();
        accessControler->tryLockSd(); // TODO

        int written = transferState->openFile.write(buf, rd);

        int tries = 1;
        while (written == 0 && tries <= 5) {
            transferState->openFile.close();
            // TODO check file mode (was write)
            transferState->openFile = SD.open(transferState->openFilePath, FILE_APPEND);
            transferState->openFile.seek(position);
            written = transferState->openFile.write(buf, rd);
            tries++;
        }

        accessControler->unlockSD();

        if (written == 0) {
            return false;
        }
    }
    return true;
}

void FTPDataProcessor::handleFailedTransfer(Session* session) {
    session->getCommandSocket()->print(
        ResponseMessage("451", "Requested action aborted: error in processing received data; Closing data connection").encode());

    if (session->getTransferState()->status == READ_IN_PROGRESS) {
        accessControler->finishedReading(session);
    }
    else if (session->getTransferState()->status == WRITE_IN_PROGRESS) {
        accessControler->finishedWriting(session);
    }
    session->cleanupTransfer();
}


void FTPDataProcessor::handleDataTransfer(Session* session) {
    TransferState* transferState = session->getTransferState();

    if (!transferState->dataSocket.connected()) {
        if (transferState->status == READ_IN_PROGRESS) {
            // peer closed connection when we still wanted to send data
            session->cleanupTransfer();
            accessControler->finishedReading(session);
            Serial.println("WARN: peer closed connection");
            return;
        }
        else if (transferState->status == WRITE_IN_PROGRESS) {
            transferState->status = FINISHED;
            accessControler->finishedWriting(session);
        }
    }

    if (transferState->status == READ_IN_PROGRESS) {
        bool success = sendDataChunk(transferState);
        if (transferState->status == FINISHED)
            accessControler->finishedReading(session);

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

void FTPDataProcessor::generateIV(uint8_t iv[IV_LEN]) {
    // TODO find secure rng func
    for (int i = 0; i < IV_LEN; i++) {
        iv[i] = (uint8_t)random(0, 256);
    }
}

AccessControler* FTPDataProcessor::getAccessControler() {
    return accessControler;
}
