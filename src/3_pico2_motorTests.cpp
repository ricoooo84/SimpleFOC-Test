// ⚠️⚠️ TRY NOT TO SWITCH DIRECTIONS ABRUPTLY W/O STOPPING FIRST

#include <Arduino.h>
#include "SimpleDCMotor.h"
#include "Adafruit_VL53L0X.h"

const uint8_t SDApin0 = 16, SCLpin0 = 17, INTpin = 18, XSHUTpin = 19, tofAddr = 0x29;
const uint8_t IN1pin = 2, IN2pin = 3, ENpin = 0;
const uint8_t testPin = 28;

DCDriver2PWM driver = DCDriver2PWM(IN1pin, IN2pin, ENpin);
Adafruit_VL53L0X sensor;

// will motor even move
// result: yes
void sanityTest() {
    digitalWrite(testPin, HIGH);
    delay(1000);
    digitalWrite(testPin,LOW);

    driver.setPwm(driver.voltage_limit);
    delay(3000);
    driver.setPwm(0.0f);
    delay(3000);
    driver.setPwm(-driver.voltage_limit);
    delay(3000);
    driver.setPwm(0.0f);
}

// observe how fast it responds to voltage, if theres "stall" voltage
// result: motor needs ~4 V to move
void stepTest() {
    float voltage;
    digitalWrite(testPin,HIGH);
    delay(1000);
    digitalWrite(testPin,LOW);

    for (voltage = 0; voltage <= driver.voltage_limit; voltage += 2) {
        Serial.println(voltage);
        driver.setPwm(voltage);
        delay(4000);
    }

    for (voltage = driver.voltage_limit; voltage >= -driver.voltage_limit; voltage -= 2) {
        Serial.println(voltage);
        driver.setPwm(voltage);
        delay(4000);
    }

    delay(4000);
}

// try to simulate acceleration
// result: i didnt try this
void rampTest() {
    float voltage;
    digitalWrite(testPin,HIGH);
    delay(1000);
    digitalWrite(testPin, LOW);

    for (voltage = 0; voltage <= driver.voltage_limit; ++voltage) {
        Serial.println(voltage);
        driver.setPwm(voltage);
        delay(250);
    }

    for (voltage = driver.voltage_limit; voltage >= 0; --voltage) {
        Serial.println(voltage);
        driver.setPwm(voltage);
        delay(250);
    }

    delay(1000);
}

// control motor with serial commands
// result: yes
void serialControl() {
    float voltage = 0;
    digitalWrite(testPin,HIGH);
    delay(1000);
    digitalWrite(testPin, LOW);

    if (Serial.available()) {
        voltage = Serial.readStringUntil('\n').toFloat();
        driver.setPwm(voltage);
        Serial.print("setPwm: ");
        Serial.println(voltage);
        delay(100);
    }
}

// control motor with time of flight input
// result: yes
void tofSanity() {
    uint16_t distance = 0;
    VL53L0X_RangingMeasurementData_t measure;
    sensor.getSingleRangingMeasurement(&measure);
    distance = measure.RangeMilliMeter;

    Serial.println(distance);

    if (distance < 150) {
        driver.setPwm(12.0f);
    } else {
        driver.setPwm(0.0f);
    }
    delay(50);
}

// stop motor if close distances detected
// result: yes, this was a lot smoother than expected
void tofProximity() {
    uint16_t distance = 0;
    VL53L0X_RangingMeasurementData_t measure;
    sensor.getSingleRangingMeasurement(&measure);
    distance = measure.RangeMilliMeter;

    Serial.println(distance);

    float voltage = (distance - 50)/250.0*driver.voltage_limit;
    driver.setPwm(voltage);

    delay(50);
}

// stop motor if no heartbeat detected
void watchdogTest() {

}

void setup() {
    pinMode(testPin, OUTPUT);

    Serial.begin(115200);

    digitalWrite(testPin, HIGH);
    delay(1000);
    Wire.setSDA(SDApin0);
    Wire.setSCL(SCLpin0);
    Wire.begin();

    if (sensor.begin(tofAddr, false, &Wire)) {
        digitalWrite(testPin, LOW);
        delay(1000);
    }

    driver.voltage_power_supply = 18.0f;
    driver.voltage_limit = 18.0f;
    driver.pwm_frequency = 20000;
    driver.init();
    driver.enable();

    delay(5000);
}

void loop() {
    // sanityTest();
    // stepTest();
    // rampTest();
    serialControl();
    // tofSanity();
    // tofProximity();
    // watchdogTest();
}
