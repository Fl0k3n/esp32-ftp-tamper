#include "FTPServiceHandler.h"

FTPServiceHandler::FTPServiceHandler() {

}

bool FTPServiceHandler::canHandle(CommandMessage*) {
    return false;
}

void FTPServiceHandler::handleMessage(CommandMessage*, Session*) {

}