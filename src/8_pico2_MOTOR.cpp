#include <Arduino.h>
#include <Wire.h>
#include "SimpleDCMotor.h"

// THINGS TO CHANGE:
// supplied voltage variable (driver.voltage_limit/power_supply)
// GPIO pin variables (below)
// led blink for debug in setup()

const uint8_t IN1pin = 2, IN2pin = 3, ENpin = 0; // driver 1
const uint8_t IN1pin1 = 4, IN2pin1 = 5, ENpin1 = 1; // driver 2
const uint8_t SDApin = 20, SCLpin = 21, picoAddr = 0x08; // i2c
const uint8_t onboardLED = 25; // status

unsigned long lastMsgTime;
const unsigned long WATCHDOG_TIMEOUT = 500; // ms, use millis()
const uint8_t delayMs = 20;

DCDriver2PWM driver = DCDriver2PWM(IN1pin, IN2pin, ENpin), driver2 = DCDriver2PWM(IN1pin1, IN2pin1, ENpin1);

void setDrive(float L, float R) {
    // NOTE: motor has a minimum voltage needed for motion
    // Values near 0 may not move the motors

    float left = L/127.0;   // this formula can exceed < -1 if input is -128, but it will be bounded by setPwm()
    float right = R/127.0;
    driver.setPwm(driver.voltage_limit*left);
    driver2.setPwm(driver2.voltage_limit*right);
}

// Comm packet: LEFT (-128 to 127) | RIGHT (-128 to 127) | CHECKSUM (xor) 
void receivePacket(int bytes) {
    if (Wire.available() < 3) return;

    lastMsgTime = millis();

    // read bytes into 3 bytes
    int8_t packet[3];
    for (int i = 0; i < 3; i++) packet[i] = Wire.read();
    
    // check checksum: onboardLED on/blinking depending on verification
    // cast to unsigned int for XOR
    if (((uint8_t)packet[0] ^ (uint8_t)packet[1]) != (uint8_t)packet[2]) {
        digitalWrite(onboardLED, LOW);
    } else {
        digitalWrite(onboardLED, HIGH);
        setDrive(packet[0],packet[1]);
    }
}

void setup() {
    pinMode(onboardLED,OUTPUT);

    digitalWrite(onboardLED, HIGH); // debug 
    delay(500);

    Wire.setSCL(SCLpin);
    Wire.setSDA(SDApin);
    Wire.begin(picoAddr);
    Wire.onReceive(receivePacket);

    digitalWrite(onboardLED, LOW); // debug 
    delay(500);

    driver.voltage_power_supply = 18.0f;    driver2.voltage_power_supply = 18.0f;
    driver.voltage_limit = 18.0f;           driver2.voltage_limit = 18.0f;
    driver.pwm_frequency = 20000;           driver2.pwm_frequency = 20000;
    driver.init();                          driver2.init();
    driver.enable();                        driver2.enable();

    digitalWrite(onboardLED, HIGH); // debug 
    delay(500);

    lastMsgTime = millis();

    digitalWrite(onboardLED, LOW); // debug 
    delay(500);
}

void loop() {

    if (millis() - lastMsgTime > WATCHDOG_TIMEOUT) {
        setDrive(0,0);
        digitalWrite(onboardLED, LOW);
    }

    delay(delayMs);
}