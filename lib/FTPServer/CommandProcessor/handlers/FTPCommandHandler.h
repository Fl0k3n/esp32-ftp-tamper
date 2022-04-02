#ifndef FTP_COMMAND_HANDLER
#define FTP_COMMAND_HANDLER

#include <Arduino.h>
#include "Session.h"
#include "ResponseMessage.h"


class FTPCommandHandler {
protected:
    bool canHandleCommand(String cmd, String acceptableCommands[], int commandsCount) {
        for (int index = 0; index < commandsCount; index++)
            if (cmd.equalsIgnoreCase(acceptableCommands[index]))
                return true;
        return false;
    }

    void sendReply(Session* session, String replyStatus, String replyText) {
        ResponseMessage response(replyStatus, replyText);

        session->getCommandSocket()->print(response.encode());
        Serial.println("sent response");
    }
};

#endif