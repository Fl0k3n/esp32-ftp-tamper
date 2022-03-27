#ifndef FTP_SERVER
#define FTP_SERVER

class FTPServer {

private:
    // private variables and functions' definitions
    void encryptFile();
    void decryptFile();

public:
    // public variables and functions' definitions to be used in external sources
    void begin();
    void handleFTP();

};

#endif