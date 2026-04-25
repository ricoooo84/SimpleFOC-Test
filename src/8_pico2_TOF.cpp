#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_VL53L0X.h"

// THINGS TO CHANGE:
// GPIO pin variables (especially I2C)
// debug onboard LED

const uint8_t numOfSensors = 3;
Adafruit_VL53L0X sensors[numOfSensors];
volatile uint16_t valuesToSend[numOfSensors] = {0};

const uint8_t SDApin0 = 20, SCLpin0 = 21, picoAddr = 0x09; // i2c slave
const uint8_t SDApin1 = 18, SCLpin1 = 19; // i2c master
const uint8_t XSHUTpins[numOfSensors] = {14, 4, 17};
const uint8_t INTpins[numOfSensors] = {15, 3, 16};
const uint8_t onboardLED = 25; // status

const uint8_t tofAddress[numOfSensors] = {0x30, 0x31, 0x32};

void onRequest() {
    // sensor0 | sensor1 | sensor2 | checksum
    uint8_t buf[numOfSensors + 1]; // + 1 for checksum
    for (uint8_t i = 0; i < numOfSensors; ++i) {
        buf[i] = valuesToSend[i]/16; // convert to 8 bit
    }
    buf[numOfSensors] = buf[0] ^ buf[1] ^ buf[2];
    Wire.write(buf, numOfSensors + 1);
}

void setup() {
    // set GPIO pin modes
    pinMode(onboardLED, OUTPUT);
    for (uint8_t i = 0; i < numOfSensors; ++i) {
        pinMode(XSHUTpins[i], OUTPUT);
        pinMode(INTpins[i], INPUT);
    }

    digitalWrite(onboardLED, HIGH); //debug
    Wire.setSDA(SDApin0);
    Wire.setSCL(SCLpin0);
    Wire.begin(picoAddr);
    Wire.onRequest(onRequest);
    Wire1.setSDA(SDApin1);
    Wire1.setSCL(SCLpin1);
    Wire1.begin();
    delay(500); //debug

    digitalWrite(onboardLED,LOW); //debug
    // handle tof addresses
    for (uint8_t i = 0; i < numOfSensors; i++) {
        digitalWrite(XSHUTpins[i], LOW);
    }
    delay(10);
    for (uint8_t i = 0; i < numOfSensors; i++) {
        digitalWrite(XSHUTpins[i], HIGH);
        delay(10);
        if (!sensors[i].begin(tofAddress[i], false, &Wire1)) {
            // stop program if sensors not initialized
            while (true) { digitalWrite(onboardLED, HIGH); delay(100); digitalWrite(onboardLED, LOW); delay(100); }
        }
    }
    delay(500); //debug

    digitalWrite(onboardLED, HIGH); //debug
    for (uint8_t i = 0; i < numOfSensors; i++) {
        sensors[i].startRangeContinuous();
    }
    delay(500); //debug
}

void loop() {
    for (uint8_t i = 0; i < numOfSensors; i++) {
        if (digitalRead(INTpins[i]) == LOW) {
            VL53L0X_RangingMeasurementData_t dist;
            sensors[i].getRangingMeasurement(&dist);
            uint16_t raw = dist.RangeMilliMeter;
            valuesToSend[i] = (raw > 4080) ? 255 : raw / 16; // 255 = error/out of range
        }
    }
}