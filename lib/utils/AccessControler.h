#ifndef ACCESS_CONTROLER
#define ACCESS_CONTROLER

#include <Arduino.h>

class AccessControler {
private:
    SemaphoreHandle_t ftpLock;
    SemaphoreHandle_t sdLock;

public:
    volatile int readers;
    volatile int writers;

    AccessControler() {
        readers = writers = 0;
        ftpLock = xSemaphoreCreateBinary();
        sdLock = xSemaphoreCreateBinary();
        xSemaphoreGive(ftpLock);
        xSemaphoreGive(sdLock);
    }

    // reentrant lock
    bool canRead(Session* session) {
        if (session->readLockCount > 0) {
            session->readLockCount++;
            return true;
        }

        // TODO finite wait
        if (xSemaphoreTake(ftpLock, portMAX_DELAY) != pdTRUE)
            return false;

        // allow thread to r/w if its the only writer
        if (session->writeLockCount == 0 && writers > 0) {
            xSemaphoreGive(ftpLock);
            return false;
        }

        session->readLockCount++;
        readers++;
        xSemaphoreGive(ftpLock);
        return true;
    }


    // reentrant lock
    bool canWrite(Session* session) {
        if (session->writeLockCount > 0) {
            session->writeLockCount++;
            return true;
        }
        // TODO finite wait
        if (xSemaphoreTake(ftpLock, portMAX_DELAY) != pdTRUE)
            return false;

        // allow thread to r/w if its the only reader and writer
        if ((session->readLockCount > 0 && readers > 1) ||
            (session->readLockCount == 0 && readers > 0) ||
            writers > 0
            ) {
            xSemaphoreGive(ftpLock);
            return false;
        }

        session->writeLockCount++;
        writers++;
        xSemaphoreGive(ftpLock);
        return true;
    }

    void finishedReading(Session* session) {
        session->readLockCount--;
        if (session->readLockCount == 0) {
            xSemaphoreTake(ftpLock, portMAX_DELAY); // TODO assert taken
            readers--;
            xSemaphoreGive(ftpLock);
        }
    }

    void finishedWriting(Session* session) {
        session->writeLockCount--;
        if (session->writeLockCount == 0) {
            xSemaphoreTake(ftpLock, portMAX_DELAY); // TODO assert taken
            writers--;
            xSemaphoreGive(ftpLock);
        }
    }

    bool tryLockSd() {
        return xSemaphoreTake(sdLock, portMAX_DELAY) == pdTRUE; // TODO finite wait
    }

    void unlockSD() {
        xSemaphoreGive(sdLock);
    }

    void cleanSessionLocks(Session* session) {
        if (session->readLockCount > 0 || session->writeLockCount > 0) {
            xSemaphoreTake(ftpLock, portMAX_DELAY); // TODO assert taken
            if (session->readLockCount > 0)
                readers--;
            if (session->writeLockCount > 0)
                writers--;
            xSemaphoreGive(ftpLock);
        }
    }

};

#endif