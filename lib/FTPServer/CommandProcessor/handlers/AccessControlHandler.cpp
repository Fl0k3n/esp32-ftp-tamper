#include "AccessControlHandler.h"

const String AccessControlHandler::canHandleCmds[canHandleCmdsNumber] = { "USER", "PASS", "AUTH", "CWD", "CDUP", "REIN", "QUIT" };

AccessControlHandler::AccessControlHandler(String username, String password)
    : username(username), password(password) {}


/*
 *  5.3. COMMANDS section ->    The command codes are four or fewer alphabetic characters.
 *                              Upper and lower case alphabetic characters are to be treated
 *                              identically
**/
bool AccessControlHandler::canHandle(CommandMessage* msg) {
    String cmd = msg->command;

    for (int index = 0; index < canHandleCmdsNumber; index++)
        if (cmd.equalsIgnoreCase(canHandleCmds[index]))
            return true;
    return false;
}

void AccessControlHandler::sendReply(Session* session, String replyStatus, String replyText) {
    ResponseMessage response(replyStatus, replyText);

    session->getCommandSocket()->print(response.encode());
    Serial.println("sent response");
}


void AccessControlHandler::handleMessage(CommandMessage* msg, Session* session) {
    Serial.println("Access control handler: handling ");
    msg->print();

    if (session->getSessionStatus() == LOGOUT) {
        sendReply(session, "421", "Server is about to close the connection with this client.");
        return;
    }

    if (session->getSessionStatus() == REINITIALIZATION) {
        sendReply(session, "421", "Server is waiting to reinitialize the connection");
        return;
    }

    String command = msg->command;
    command.toUpperCase();

    if (command == "USER") {
        handleUserCmd(msg, session);
    }
    else if (command == "PASS") {
        handlePasswordCmd(msg, session);
    }
    else if (command == "AUTH") {
        handleAuthCmd(msg, session);
    }
    else if (command == "QUIT") {
        handleQuitCmd(msg, session);
    }
    else if (command == "CWD") {
        handleCwdCmd(msg, session);
    }
    else if (command == "CDUP") {
        handleCdupCmd(msg, session);
    }
    else if (command == "REIN") {
        handleReinitCmd(msg, session);
    }
}

void AccessControlHandler::handleAuthCmd(CommandMessage* msg, Session* session) {
    sendReply(session, "502", "Security mechanism not implemented. Proceed to log in using USER and PASS");
}

void AccessControlHandler::handleUserCmd(CommandMessage* msg, Session* session) {
    if (session->getSessionStatus() == AWAIT_COMMAND) {
        sendReply(session, "230", "User already logged in, proceed.");
        return;
    }

    if (!username.equals(msg->data)) {
        Serial.println(username);
        Serial.println(msg->data);
        sendReply(session, "530", "User not found");
        return;
    }

    Serial.println("got correct username as expected");

    sendReply(session, "331", "User name ok, need password");
    session->setSessionStatus(AWAIT_PASSWORD);
}

void AccessControlHandler::handlePasswordCmd(CommandMessage* msg, Session* session) {
    if (session->getSessionStatus() == AWAIT_COMMAND) {
        sendReply(session, "230", "User already logged in, proceed.");
        return;
    }

    if (session->getSessionStatus() != AWAIT_PASSWORD) {
        sendReply(session, "503", "PASS was not expected now");
        return;
    }

    if (!password.equals(msg->data)) {
        sendReply(session, "530", "Invalid password");
    }

    Serial.println("got correct password as expected");

    sendReply(session, "230", "Logged in successfully");
    session->setSessionStatus(AWAIT_COMMAND);
}

void AccessControlHandler::handleQuitCmd(CommandMessage* msg, Session* session) {
    Serial.println("Preparing to end connection with the client...");
    session->setSessionStatus(LOGOUT);
}

void AccessControlHandler::handleReinitCmd(CommandMessage* msg, Session* session) {
    Serial.println("Preparing to reinitialize connection");
    session->setSessionStatus(REINITIALIZATION);
}

void AccessControlHandler::handleCwdCmd(CommandMessage* msg, Session* session) {
    if (session->getSessionStatus() != AWAIT_COMMAND) {
        sendReply(session, "530", "Not logged in.");
        return;
    }

    Serial.print("Old working directory: ");
    Serial.println(session->getWorkingDirectory());

    String newWorkingDirectory = changeDirectory(session->getWorkingDirectory(), msg->data);
    // TODO: check if newWorkingDirectory exists on SD, if not, return 550, Directory not found
    session->setWorkingDirectory(newWorkingDirectory);

    Serial.print("New working directory: ");
    Serial.println(session->getWorkingDirectory());

    String replyStatus = "250";
    if (msg->command == "CDUP")
        replyStatus = "200";

    sendReply(session, replyStatus, "Command okay, completed.");
}

void AccessControlHandler::handleCdupCmd(CommandMessage* msg, Session* session) {
    CommandMessage cmdMessage("CDUP", "..");
    handleCwdCmd(&cmdMessage, session);
}

String AccessControlHandler::changeDirectory(String currentWorkingDirectory, String path) {
    if (path == ".")
        return currentWorkingDirectory;

    if (path == "/")
        return "/";

    if (path == "..") {
        if (currentWorkingDirectory == "/")
            return "/";

        int end = currentWorkingDirectory.lastIndexOf('/');
        if (end == 0)
            end = 1;

        return currentWorkingDirectory.substring(0, end);
    }

    if (path.charAt(0) == '/')
        return path;

    if (currentWorkingDirectory == "/") {
        currentWorkingDirectory.concat(path);
        return currentWorkingDirectory;
    }

    currentWorkingDirectory.concat("/" + path);
    return currentWorkingDirectory;
}