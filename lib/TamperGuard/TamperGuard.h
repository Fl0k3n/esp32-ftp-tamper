#ifndef TAMPER_GUARD
#define TAMPER_GUARD

#include <Arduino.h>
#include "EmailService.h"
#include "PreferencesHandler.h"
#include "StateSignaler.h"
#include "TimeoutHandler.h"

#define INTRUSION_QUEUE_SIZE 16

#define MAX_PIN_RETRIES 5
#define CORRECT_PIN 1
#define INCORRECT_PIN 0

#define SECURE_MODE_TIMEOUT_MS (3 * 60 * 1000) // 3 minutes

typedef enum tamper_sensor_t {
    MOTION,  // data is NULL
    LIGHT,   // data is NULL
    KEYPAD,  // data is int which equals 1 iff pin was entered correctly
    TIMER,   // data is NULL
} tamper_sensor_t;


enum SecurityMode {
    SECURE = 0,
    UNSECURE = 1
};

typedef struct Intrusion {
    tamper_sensor_t detector;
    TickType_t timestamp;
    void* sensorData;
} Intrusion;


const Intrusion TIMER_INTRUSION = {
    .detector = TIMER
};

class TamperGuard {
private:
    TimeoutHandler timeoutHandler;
    xTaskHandle timeoutTask;
    xQueueHandle intrusionQueue;
    TimeoutArgs timeoutArgs;

    EmailService* emailService;
    PreferencesHandler* prefs;
    StateSignaler* signaler;

    xTaskHandle* tasksToStop;
    int tasksToStopCount;

    SecurityMode mode;
    unsigned int invalidPinRetries;

    void handleIntrusion(Intrusion);
    void handleKeypadIntrusion(Intrusion);

    void handleSecurityBreach();

    void switchToSecureMode();
    void switchToUnsecureMode(bool shouldStopTimer);
public:
    TamperGuard(EmailService*, PreferencesHandler*, StateSignaler*, xTaskHandle*, int);
    void registerIntrusion(Intrusion);
    void enterSecurityMode(SecurityMode);
    void run();
};



#endif