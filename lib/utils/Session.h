#ifndef SESSION
#define SESSION

#include <WiFi.h>
#include "TransferState.h"

enum SessionStatus {
    AWAIT_USERNAME = 0,
    AWAIT_PASSWORD = 1,
    AWAIT_COMMAND = 2,
    LOGOUT = 3,
    REINITIALIZATION = 4
};


enum SessionMode {
    ACTIVE = 0,
    PASSIVE = 1
};


class Session {
private:
    WiFiClient* commandSocket;
    WiFiServer dataServerSocket;

    TransferState transferState;

    SessionStatus sessionStatus;
    String workingDirectory;
    String messageBuff;

    String fileToRename;

    bool isListenningForData;
    uint16_t dataPort;

public:
    // reentrant locks
    unsigned int readLockCount;
    unsigned int writeLockCount;
    SessionMode mode;

    Session(WiFiClient* commandSock, ChaCha* cipher);

    SessionStatus getSessionStatus();
    void setSessionStatus(SessionStatus);

    WiFiClient* getCommandSocket();

    // TODO replace this with char* buff, with predefined max length to avoid dynamic memory allocation
    String getMessageBuff();
    String getFileToRename();
    void setMessageBuff(String buff);
    void clearMessageBuff();
    void clearFileToRename();
    void setFileToRename(String);

    String getWorkingDirectory();
    void setWorkingDirectory(String);

    TransferState* getTransferState();

    void init();
    bool canBeLoggedOut();

    bool shouldListenForDataConnections();
    void startListenningForDataConnection(uint16_t port);
    void stopListenningForDataConnection();

    WiFiServer* getDataServerSocket();

    void cleanupTransfer();
};

#endif