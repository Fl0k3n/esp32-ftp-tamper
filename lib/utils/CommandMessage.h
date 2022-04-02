#ifndef COMMAND_MESSAGE
#define COMMAND_MESSAGE

#define MIN_COMMAND_LENGTH 3
#define DELIM_LENGTH 2 // length of \r\n

#include <Arduino.h>


class CommandMessage {
public:
    const String command;
    const String data;

    CommandMessage(const String command, const String data) : command(command), data(data) {}

    static CommandMessage decode(String rawMessage) {
        // not the most elegant solution but we probably shouldnt use exceptions 
        if (rawMessage.length() < MIN_COMMAND_LENGTH + DELIM_LENGTH)
            return CommandMessage("_ERR", rawMessage);

        int sp = rawMessage.indexOf(" ");
        if (sp == -1)
            sp = rawMessage.indexOf("\r");

        String command = rawMessage.substring(0, sp);
        command.toUpperCase();

        String data = rawMessage.substring(sp + 1, rawMessage.length() - DELIM_LENGTH);

        if (rawMessage.length() - sp <= DELIM_LENGTH)
            data = "";

        return CommandMessage(command, data);
    }

    void print() {
        Serial.print("Message {Command: ");
        Serial.print(command);
        Serial.print("; Data: ");
        Serial.print(data);
        Serial.println(";}");
    }
};

#endif