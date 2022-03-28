#ifndef RESPONSE_MESSAGE
#define RESPONSE_MESSAGE

#include <Arduino.h>

class ResponseMessage {
public:
    const String responseCode;
    const String responseData;

    ResponseMessage(String responseCode, String responseData)
        : responseCode(responseCode), responseData(responseData) {}

    String encode() {
        // or smth like this
        return responseCode + " " + responseData + "\r\n";
    }
};

#endif