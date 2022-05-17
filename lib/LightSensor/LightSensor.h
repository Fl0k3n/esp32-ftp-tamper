#ifndef LIGHT_SENSOR
#define LIGHT_SENSOR

#include <Arduino.h>

#define ANALOG_PIN 36
#define LIGHT_VALUES 20
#define ANOMALIES 4

class LightSensor {
private:
    int lightValuesIndex = 0;
    int anomaliesIndex = 0;
    int lightValues[LIGHT_VALUES] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    bool anomalies[ANOMALIES] = { false, false, false, false };
public:
    void resetLightValues();
    void resetAnomalies();
    int readValueFromLightSensor();
    bool processNewLightValue(int val);
    bool checkForAnomaly();
};

#endif