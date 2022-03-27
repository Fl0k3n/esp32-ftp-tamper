#ifndef FTP_SERVER
#define FTP_SERVER

#include <WiFi.h>

#include <ftp_control_process.h>
#include <ftp_command_processor.h>

class FTPServer {

public:
    FTPServer();
    void mainFtpLoop();

private:
    FTPControlProcess controlProcess;
    FTPCommandProcessor commandProcessor;
};

#endif