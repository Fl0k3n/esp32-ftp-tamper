#ifndef LIGHT_SENSOR
#define LIGHT_SENSOR

#include <Arduino.h>

#define ANALOG_PIN 36
#define LIGHT_VALUES 20
#define ANOMALIES 3

class LightSensor {
private:
    int anomalyThreshold;
    int lightValuesIndex;
    int calibratedLightValue;
    int anomaliesIndex;
    int lightValues[LIGHT_VALUES];
    bool anomalies[ANOMALIES];

    int getMeanLightValue();
public:
    LightSensor(int);
    void resetLightValues();
    void resetAnomalies();
    void calibrate();
    int readValueFromLightSensor();
    bool processNewLightValue();
    bool checkForAnomaly();
};

#endif