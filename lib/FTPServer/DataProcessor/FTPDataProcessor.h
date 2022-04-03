#ifndef FTP_DATA_PROCESSOR
#define FTP_DATA_PROCESSOR

#include "Session.h"
#include <WiFi.h>
#include <SD.h>
#include "ResponseMessage.h"

#define READ_BUFF_SIZE 512
#define WRITE_BUFF_SIZE 512

class FTPDataProcessor {
private:
    // assumes that session is ready to send data and file is open for reading/writing
    void sendDataChunk(TransferState*);
    bool receiveDataChunk(TransferState*);
public:
    // returns true if connection was successully established
    bool establishActiveSession(Session*); // connect in ACTIVE mode

    // shouldn't transfer entire file in one call, but in chunks so we can handle commands in the meantime (such as ABORT)
    void handleDataTransfer(Session*);
};


#endif
