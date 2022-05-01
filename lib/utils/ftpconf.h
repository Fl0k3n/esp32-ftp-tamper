#ifndef FTPCONF
#define FTPCONF

#define DEFAULT_CLIENT_DATA_PORT 50009 // used in ACTIVE mode when PORT command wasn't sent
#define CONTROL_SERVER_PORT 21
#define MAX_CLIENTS 5

#define FTP_BUF_SIZE 4096

#define DATA_CONNECTION_TIMEOUT_MILLIS 500

#define MIN_COMMAND_LENGTH 3
#define DELIM_LENGTH 2 // length of \r\n

#define CHACHA_ROUNDS 20
#define IV_LEN 8 // bytes
#define KEY_LEN 16

#endif