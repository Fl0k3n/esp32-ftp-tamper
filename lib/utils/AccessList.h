#ifndef ACCESS_LIST
#define ACCESS_LIST

#include "ftpconf.h"
#include <Arduino.h>


typedef struct ACLItem {
    bool taken;
    char fileMode;
    int readersCount;
    int writerCount;
    char* filePath;
} ACLItem;


class AccessList {
private:
    int clients;
    SemaphoreHandle_t aclLock;
    QueueHandle_t aclCond;
    ACLItem acls[MAX_CLIENTS];


    int findFileAccesor(char* filePath) {
        if (clients == 0)
            return -1;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (acls[i].taken && strcmp(filePath, acls[i].filePath) == 0)
                return i;
        }

        return -1;
    }

    int findFreeAcl() {
        for (int i = 0; i < MAX_CLIENTS; i++)
            if (!acls[i].taken)
                return i;
        return -1;
    }

    bool wait(int aclId, char mode) {
        xSemaphoreGive(aclLock);
        void* buff;

        // TODO timeouts
        while (true) {
            xQueueReceive(aclCond, buff, portMAX_DELAY);
            //...
        }
    }

public:
    AccessList() {
        clients = 0;
        aclLock = xSemaphoreCreateBinary();
        aclCond = xQueueCreate(MAX_CLIENTS, sizeof(void*));

        for (int i = 0; i < MAX_CLIENTS; i++) {
            acls[i].taken = false;
            acls[i].readersCount = acls[i].writersCount = 0;
        }
    }


    bool requestFileRead(char* filePath) {
        // TODO finite delay
        if (xSemaphoreTake(aclLock, portMAX_DELAY) != pdTRUE) {
            return false;
        }

        int accessor = findFileAccesor(filePath);
        if (accessor == -1) {
            int fa = findFreeAcl();
            acls[fa].taken = true;
            acls[fa].fileMode = 'r';
            acls[fa].filePath = filePath;
        }
        else if (acls[accesor].fileMode == 'w') {

        }

    }


};

#endif
