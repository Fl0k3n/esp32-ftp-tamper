#ifndef DATA_TRANSFER_STATE
#define DATA_TRANSFER_STATE
#define DEFAULT_CLIENT_DATA_PORT 50009 // TODO, used in ACTIVE mode when PORT command wasn't sent

#include <WiFi.h>
#include <SD.h>

enum TransferStatus {
    NO_TRANSFER = 0,    // dataSocket mustn't be used
    READ_IN_PROGRESS = 1,
    WRITE_IN_PROGRESS = 2,
    FINISHED = 3      // transient state, dataSocket must be closed as soon as this state is detected
};


// if you want you can refactor this with getters and setters, not sure if cpp is as strict with this as java
class TransferState {
public:
    TransferStatus status;
    WiFiClient dataSocket;

    File openFile; // valid only if isTransferInProgress()

    // address used to estb connection in ACTIVE mode
    String clientDataIP;
    uint16_t clientDataPort;

    TransferState(String clientControlIP)
        : status(NO_TRANSFER), clientDataIP(clientControlIP), clientDataPort(DEFAULT_CLIENT_DATA_PORT) {}

    ~TransferState() {
        if (dataSocket.connected())
            dataSocket.stop();
    }

    bool isDataConnectionClosed() {
        // TODO not sure how FINISHED will be handled
        return status == NO_TRANSFER;
    }

    bool isTransferInProgress() {
        return status == READ_IN_PROGRESS || status == WRITE_IN_PROGRESS;
    }

    bool isTransferFinished() {
        return status == FINISHED;
    }

    void cleanupTransfer() {
        status = NO_TRANSFER;
        dataSocket.stop();
        openFile.close();
    }
};



#endif