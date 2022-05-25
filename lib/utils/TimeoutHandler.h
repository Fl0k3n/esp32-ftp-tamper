#ifndef TIMEOUT_HANDLER
#define TIMEOUT_HANDLER

#include <Arduino.h>

#define TIMER_TASK_PRIORITY 5

typedef struct TimeoutArgs {
    QueueHandle_t callbackQueue;
    void* callbackArgs;
    int timeoutMs;
} TimeoutArgs;

class TimeoutHandler {
private:

    static void awaitTimeout(void *args) {
        TimeoutArgs* targs = (TimeoutArgs*) args;
        vTaskDelay(targs->timeoutMs / portTICK_PERIOD_MS);

        xQueueSend(targs->callbackQueue, targs->callbackArgs, 0);
        vTaskDelete(NULL);
    }


public:
    TaskHandle_t requestTimeout(TimeoutArgs* args) {
        TaskHandle_t timerHandle;

        xTaskCreate(
            awaitTimeout,
            "timer",
            512,
            args,
            TIMER_TASK_PRIORITY,
            &timerHandle            
        );

        return timerHandle;
    }

    void cancelTimeout(TaskHandle_t timer) {
        vTaskDelete(timer);
    }

    TaskHandle_t resetTimeout(TaskHandle_t prevTimer, TimeoutArgs* args) {
        cancelTimeout(prevTimer);
        return requestTimeout(args);
    }
};

#endif