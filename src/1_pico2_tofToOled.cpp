#include <Arduino.h>
#include "Wire.h"
#include "Adafruit_VL53L0X.h"

const uint8_t SDApin0 = 20, SCLpin0 = 21, picoAddr = 0x08;
const uint8_t SDApin1 = 18, SCLpin1 = 19, tofAddr = 0x29;
const uint8_t testPin = 28;

Adafruit_VL53L0X sensor;
volatile uint16_t valueToSend = 0;

void onRequest() {
    uint8_t buf[2];
    buf[0] = valueToSend & 0xFF;
    buf[1] = (valueToSend >> 8) & 0xFF;
    Wire.write(buf, 2);
}

void setup() {
    Wire.setSDA(SDApin0);
    Wire.setSCL(SCLpin0);
    Wire.begin(picoAddr);
    Wire.onRequest(onRequest);

    Wire1.setSDA(SDApin1);
    Wire1.setSCL(SCLpin1);
    Wire1.begin();

    if (sensor.begin(tofAddr, false, &Wire1)) {
        pinMode(testPin, OUTPUT);
        digitalWrite(testPin, HIGH);
    }
}

void loop() {
    VL53L0X_RangingMeasurementData_t distance;
    sensor.getSingleRangingMeasurement(&distance);
    valueToSend = distance.RangeMilliMeter;
    delay(100);
}