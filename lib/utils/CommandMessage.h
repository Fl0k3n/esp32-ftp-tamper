#ifndef COMMAND_MESSAGE
#define COMMAND_MESSAGE

#define COMMAND_LENGTH 4
#define DELIM_LENGTH 2 // length of \r\n

#include <Arduino.h>


class CommandMessage {
public:
    const String command;
    const String data;

    CommandMessage(const String command, const String data) : command(command), data(data) {}

    static CommandMessage decode(String rawMessage) {
        // not the most elegant solution but we probably shouldnt use exceptions 
        if (rawMessage.length() < COMMAND_LENGTH + DELIM_LENGTH)
            return CommandMessage("_ERR", rawMessage);

        String command = rawMessage.substring(0, COMMAND_LENGTH);
        String data = rawMessage.substring(COMMAND_LENGTH + 1, rawMessage.length() - DELIM_LENGTH);

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