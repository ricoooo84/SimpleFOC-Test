// Time of flight input inversely related with motor output
// Simulate close to something = stop

#include <Arduino.h>
#include "Adafruit_VL53L0X.h"
#include "SimpleDCMotor.h"

const uint8_t SDApin1 = 18, SCLpin1 = 19, tofAddr = 0x29;
const uint8_t IN1pin = 2, IN2pin = 3, EN1pin = 0;
const uint8_t testPin = 28;

volatile uint16_t distanceMM = 0;

Adafruit_VL53L0X sensor;
DCDriver2PWM driver = DCDriver2PWM(IN1pin, IN2pin, EN1pin);

void setup() {
    Wire1.setSDA(SDApin1);
    Wire1.setSCL(SCLpin1);
    Wire1.begin();

    if (sensor.begin(tofAddr, false, &Wire1)) {
        pinMode(testPin, OUTPUT);
        digitalWrite(testPin, HIGH);
    }

    driver.voltage_power_supply = 12.0f;
    driver.voltage_limit = 12.0f;
    driver.pwm_frequency = 20000;
    driver.enable();
    driver.init();
}

void loop() {
    VL53L0X_RangingMeasurementData_t distance;
    sensor.getSingleRangingMeasurement(&distance);
    distanceMM = distance.RangeMilliMeter;

    float voltage = (distanceMM - 50)/250.0*12.0f;
    
    driver.setPwm(voltage);
    
    Serial.print("voltage: ");
    Serial.print(voltage);
    Serial.print("\t");
    Serial.print("distance: ");
    Serial.println(distanceMM);

    delay(100);
}