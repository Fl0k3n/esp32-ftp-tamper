#include "LightSensor.h"

LightSensor::LightSensor(int anomalyThreshold): anomalyThreshold(anomalyThreshold) {
    resetLightValues();
    resetAnomalies();
    calibratedLightValue = -1;
}

void LightSensor::resetLightValues() {
    lightValuesIndex = 0;
    for (int i = 0; i < LIGHT_VALUES; i++) {
        lightValues[i] = -1;
    }
}

void LightSensor::resetAnomalies() {
    anomaliesIndex = 0;
    for (int i = 0; i < ANOMALIES; i++) {
        anomalies[i] = false;
    }
}

int LightSensor::readValueFromLightSensor() {
    return analogRead(ANALOG_PIN);
}

bool LightSensor::processNewLightValue() {
    int val = readValueFromLightSensor();
    bool anomaly = false;

    if (lightValues[LIGHT_VALUES - 1] != -1) {
        if (abs(val - calibratedLightValue) > anomalyThreshold) {
            anomaly = true;
        }

        anomalies[anomaliesIndex] = anomaly;
        anomaliesIndex = (anomaliesIndex + 1) % ANOMALIES;

        if (checkForAnomaly()) return true;
    }

    if (anomaly) return false;

    lightValues[lightValuesIndex] = val;
    lightValuesIndex = (lightValuesIndex + 1) % LIGHT_VALUES;

    return false;
}

void LightSensor::calibrate() {
    resetAnomalies();
    resetLightValues();

    for (int i=0; i<LIGHT_VALUES; i++) {
        lightValues[i] = readValueFromLightSensor();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    calibratedLightValue = getMeanLightValue();
}

bool LightSensor::checkForAnomaly() {
    for (int i = 0; i < ANOMALIES; i++) {
        if (anomalies[i] == false) return false;
    }
    return true;
}

int LightSensor::getMeanLightValue() {
      int sum = 0;
        for (int i = 0; i < LIGHT_VALUES; i++) {
            sum += lightValues[i];
        }

    return sum / LIGHT_VALUES;
}