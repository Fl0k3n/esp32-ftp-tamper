#ifndef FTP_DATA_PROCESSOR
#define FTP_DATA_PROCESSOR

#include "Session.h"
#include <WiFi.h>
#include <SD.h>
#include "ResponseMessage.h"
#include <Crypto.h>
#include <ChaCha.h>
#include "ftpconf.h"

class FTPDataProcessor {
private:
    const uint8_t* cipherKey;

    // assumes that session is ready to send data and file is open for reading/writing
    bool sendDataChunk(TransferState*);
    bool receiveDataChunk(TransferState*);

    void handleFailedTransfer(Session*);

    void generateIV(uint8_t ivBuff[IV_LEN]);

public:
    FTPDataProcessor(const uint8_t*);

    // returns true if connection was successully established
    bool establishDataConnection(Session*);
    // shouldn't transfer entire file in one call, but in chunks so we can handle commands in the meantime (such as ABORT)
    void handleDataTransfer(Session*);

    bool prepareCipher(Session*, String, File*);

    bool assertValidCipherConfig();
};


#endif
