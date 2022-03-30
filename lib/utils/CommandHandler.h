#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include "CommandMessage.h"

class CommandHandler {
protected:
    String* cmds;
public:
    CommandHandler() { cmds = NULL; }
    CommandHandler(String* c) : cmds(c) {};
    void setCmds(String* c) {
        cmds = c;
    }
    bool canHandle(CommandMessage* msg) {
        if (cmds == NULL) return false;
        String cmd = msg->command;

        Serial.println("yay");

        for (int i = 0; i < sizeof(cmds) / sizeof(String); i++)
            if (cmd.equalsIgnoreCase(cmds[i])) return true;
        return false;
    }
};

#endif