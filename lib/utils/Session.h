#ifndef SESSION
#define SESSION

#include <WiFi.h>

enum SessionStatus {
    AWAIT_USERNAME = 0,
    AWAIT_PASSWORD = 1,
    AWAIT_COMMAND = 2,
    LOGOUT = 3,
    REINITIALIZATION = 4
};

enum TransferStatus {
    NO_TRANSFER = 0,
    IN_PROGRESS = 1
};

class Session {
private:
    WiFiClient* commandSocket;
    WiFiClient* dataSocket; // not used yet

    SessionStatus sessionStatus;
    TransferStatus transferStatus;
    String workingDirectory;
    String messageBuff;

public:
    Session(WiFiClient* commandSock);
    ~Session();

    SessionStatus getSessionStatus();
    void setSessionStatus(SessionStatus);
    TransferStatus getTransferStatus();
    void setTransferStatus(TransferStatus);
    WiFiClient* getCommandSocket();
    WiFiClient* getDataSocket();

    String getMessageBuff();
    void setMessageBuff(String buff);
    void clearMessageBuff();

    String getWorkingDirectory();
    void setWorkingDirectory(String);

    void init();
    bool waitingToLogOutAndNoTranfser();
};

#endif