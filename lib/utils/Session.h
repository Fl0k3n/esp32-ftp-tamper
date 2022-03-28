#include <WiFi.h>

enum SessionStatus {
    AWAIT_USERNAME = 0,
    AWAIT_PASSWORD = 1,
    AWAIT_COMMAND = 2
};



class Session {
private:
    WiFiClient* commandSocket;
    WiFiClient* dataSocket;

    SessionStatus sessionStatus;
    String workingDirectory;
    String messageBuff;

public:
    Session(WiFiClient* commandSock);
    ~Session();

    SessionStatus getStatus();
    WiFiClient* getCommandSocket();

    String getMessageBuff();
    void setMessageBuff(String buff);
    void clearMessageBuff();
};