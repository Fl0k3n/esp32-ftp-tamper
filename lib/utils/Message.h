#include <WiFi.h> // TODO no idea which library defines String class, but this one works


class Message {
public:
    const String command;
    const String data;

    Message(const String command, const String data) : command(command), data(data) {}

    static Message buildFromString(String rawMessage) {
        // extract first characters as command, rest should be args
        // should throw some error if invalid input, or validate it elsewhere 

        return Message("PLACEHOLDER", rawMessage);
    }

    void print() {
        Serial.print("Message {Command: ");
        Serial.print(command);
        Serial.print("; Data: ");
        Serial.print(data);
        Serial.println(";}");
    }
};