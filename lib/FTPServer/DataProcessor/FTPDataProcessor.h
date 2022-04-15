#ifndef FTP_DATA_PROCESSOR
#define FTP_DATA_PROCESSOR

#include "Session.h"
#include <WiFi.h>
#include <SD.h>
#include "ResponseMessage.h"
#include <Crypto.h>
#include <ChaCha.h>

#define READ_BUFF_SIZE 4096
#define WRITE_BUFF_SIZE 4096

class FTPDataProcessor {
private:
    ChaCha chacha;

    const uint8_t* cipherKey;
    int cipherKeyLen;

    const uint8_t* iv;
    int ivLen;
    
    // assumes that session is ready to send data and file is open for reading/writing
    bool sendDataChunk(TransferState*);
    bool receiveDataChunk(TransferState*);

    void handleFailedTransfer(Session*);
public:
    FTPDataProcessor(ChaCha, const uint8_t*, const uint8_t*, const int, const int);

    // returns true if connection was successully established
    bool establishDataConnection(Session*);
    // shouldn't transfer entire file in one call, but in chunks so we can handle commands in the meantime (such as ABORT)
    void handleDataTransfer(Session*);

    bool prepareCipher();
};


#endif
