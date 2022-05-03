#ifndef TAMPER_GUARD
#define TAMPER_GUARD

#include <Arduino.h>
#include "EmailService.h"

#define INTRUSION_QUEUE_SIZE 16

#define MAX_PIN_RETRIES 5
#define CORRECT_PIN 1
#define INCORRECT_PIN 0

typedef enum tamper_sensor_t {
    MOTION, // data is NULL
    LIGHT,  // data is NULL
    KEYPAD  // data is int which equals 1 iff pin was entered correctly
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


class TamperGuard {
private:
    xQueueHandle intrusionQueue;
    EmailService* emailService;
    SecurityMode mode;
    unsigned int invalidPinRetries;

    void handleIntrusion(Intrusion);
    void handleKeypadIntrusion(Intrusion);
public:
    TamperGuard(EmailService*);
    void registerIntrusion(Intrusion);
    void enterSecurityMode(SecurityMode);
    void run();
};



#endif