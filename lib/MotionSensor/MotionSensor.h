#ifndef MOTION_SENSOR
#define MOTION_SENSOR

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define MPU_READING_BUFF_SIZE 16

class MPUSensorReading {
public:
    float accX, accY, accZ;
    float gyroX, gyroY, gyroZ;

    MPUSensorReading() : MPUSensorReading(0, 0, 0, 0, 0, 0) {
    }

    MPUSensorReading(float accX, float accY, float accZ, float gyroX, float gyroY, float gyroZ) {
        this->accX = accX;
        this->accY = accY;
        this->accZ = accZ;
        this->gyroX = gyroX;
        this->gyroY = gyroY;
        this->gyroZ = gyroZ;
    }

    MPUSensorReading(const MPUSensorReading& other)
        :MPUSensorReading(other.accX, other.accY, other.accZ, other.gyroX, other.gyroY, other.gyroZ) {
    }

    MPUSensorReading& operator=(const MPUSensorReading& other) {
        this->accX = other.accX;
        this->accY = other.accY;
        this->accZ = other.accZ;
        this->gyroX = other.gyroX;
        this->gyroY = other.gyroY;
        this->gyroZ = other.gyroZ;

        return *this;
    }

    void print() {
        Serial.printf("accelero: x=%f  |  y=%f  |  z=%f\tgyro: x=%f  |  y=%f  |  z=%f\n",
            accX, accY, accZ, gyroX, gyroY, gyroZ);
    }
};



class MotionSensor {
private:
    Adafruit_MPU6050 mpu;
    MPUSensorReading idle;
    MPUSensorReading measured;
    MPUSensorReading buff[MPU_READING_BUFF_SIZE];
    int nextReadPtr;
    float acceleroThreshold, gyroThreshold;

    void bufferReading(MPUSensorReading& reading) {
        buff[nextReadPtr] = reading;
        nextReadPtr = (nextReadPtr + 1) % MPU_READING_BUFF_SIZE;
    }

    MPUSensorReading computeAverageReading() {
        return computeAvgReadingOfLast(MPU_READING_BUFF_SIZE);
    }

    MPUSensorReading computeAvgReadingOfLast(int n) {
        float accX = 0, accY = 0, accZ = 0;
        float gyroX = 0, gyroY = 0, gyroZ = 0;

        for (int i = 0, rPtr = (nextReadPtr - 1 + MPU_READING_BUFF_SIZE) % MPU_READING_BUFF_SIZE; i < n; i++) {
            accX += buff[rPtr].accX;
            accY += buff[rPtr].accY;
            accZ += buff[rPtr].accZ;

            gyroX += buff[rPtr].gyroX;
            gyroY += buff[rPtr].gyroY;
            gyroZ += buff[rPtr].gyroZ;
            rPtr = (rPtr - 1 + MPU_READING_BUFF_SIZE) % MPU_READING_BUFF_SIZE;
        }

        return MPUSensorReading(accX / n, accY / n, accZ / n, gyroX / n, gyroY / n, gyroZ / n);
    }

    float rDiff(float x, float real) {
        float zero = 1e-6;
        return abs(real) < zero ? min(1.0f, abs(x)) : abs((x - real) / real);
    }

    MPUSensorReading computeRelativeDiff(MPUSensorReading& rd1, MPUSensorReading& rd2) {
        return MPUSensorReading(
            rDiff(rd1.accX, rd2.accX),
            rDiff(rd1.accY, rd2.accY),
            rDiff(rd1.accZ, rd2.accZ),
            rDiff(rd1.gyroX, rd2.gyroX),
            rDiff(rd1.gyroY, rd2.gyroY),
            rDiff(rd1.gyroZ, rd2.gyroZ)
        );
    }

    MPUSensorReading computeAbsoluteDiff(MPUSensorReading& rd1, MPUSensorReading& rd2) {
        return MPUSensorReading(
            abs(rd1.accX - rd2.accX),
            abs(rd1.accY - rd2.accY),
            abs(rd1.accZ - rd2.accZ),
            abs(rd1.gyroX - rd2.gyroX),
            abs(rd1.gyroY - rd2.gyroY),
            abs(rd1.gyroZ - rd2.gyroZ)
        );
    }

    bool anyDimGreaterThan(MPUSensorReading& reading, float x) {
        return anyAccelerationGreaterThan(reading, x) || anyGyroGreaterThan(reading, x);
    }

    bool anyAccelerationGreaterThan(MPUSensorReading& reading, float x) {
        return reading.accX > x || reading.accY > x || reading.accZ > x;
    }

    bool anyGyroGreaterThan(MPUSensorReading& reading, float x) {
        return reading.gyroX > x || reading.gyroY > x || reading.gyroZ > x;
    }

public:

    bool init(float acceleroThreshold, float gyroThreshold) {
        if (!this->mpu.begin()) {
            return false;
        }

        this->mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
        this->mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
        this->mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
        this->mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        this->acceleroThreshold = acceleroThreshold;
        this->gyroThreshold = gyroThreshold;

        return true;
    }

    void calibrate() {
        idle = MPUSensorReading(); // reset idle state

        Serial.println("Calibrating MPU, please don't move the board");
        vTaskDelay(1500 / portTICK_PERIOD_MS);

        while (true) {
            buff[0] = readSensorState();
            nextReadPtr = 1;
            int i;

            for (i = 1; i < MPU_READING_BUFF_SIZE; i++) {
                vTaskDelay(200 / portTICK_PERIOD_MS);
                MPUSensorReading reading = readSensorState();
                bufferReading(reading);

                MPUSensorReading diff = computeAbsoluteDiff(buff[i - 1], reading);

                if (anyGyroGreaterThan(diff, gyroThreshold / 2) || anyAccelerationGreaterThan(diff, acceleroThreshold / 2)) {
                    Serial.println("don't move the board!");
                    break;
                }
            }

            if (i == MPU_READING_BUFF_SIZE)
                break;
        }

        idle = computeAverageReading();

        Serial.println("Calibrated!");
    }

    MPUSensorReading readSensorState() {
        sensors_event_t acc, gyro, temp;
        mpu.getEvent(&acc, &gyro, &temp);

        return MPUSensorReading(
            acc.acceleration.x,
            acc.acceleration.y,
            acc.acceleration.z,
            gyro.gyro.x,
            gyro.gyro.y,
            gyro.gyro.z
        );
    };

    bool isMovementDetected() {

        MPUSensorReading reading = readSensorState();
        MPUSensorReading absoluteDiff = computeAbsoluteDiff(reading, idle);
        if (anyAccelerationGreaterThan(absoluteDiff, acceleroThreshold)) {
            return true;
        }

        if (anyGyroGreaterThan(absoluteDiff, gyroThreshold)) {
            return true;
        }

        return false;
    }

};
#endif