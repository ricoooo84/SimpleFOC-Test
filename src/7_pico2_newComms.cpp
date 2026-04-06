#include <Arduino.h>
#include <Wire.h>
#include "SimpleDCMotor.h"

const uint8_t IN1pin = 2, IN2pin = 3, ENpin = 0;
const uint8_t IN1pin1 = 4, IN2pin1 = 5, ENpin1 = 1;
const uint8_t SDApin = 20, SCLpin = 21, picoAddr = 0x08;
const uint8_t greenLED = 15, yellowLED = 17, redLED = 16;

unsigned long lastMsg;
const unsigned long WATCHDOG_TIMEOUT = 500; // ms, use millis()
uint8_t delayMs = 20;

DCDriver2PWM driver = DCDriver2PWM(IN1pin, IN2pin, ENpin), driver2 = DCDriver2PWM(IN1pin1, IN2pin1, ENpin1);

void setDrive(float L, float R) {
    // TODO: might need to account for starting voltage of the motor
    // If so, map "0" at ±4 V (whatever it is), prob a formula

    float left = L/127.0;   // goes past -1 if input is -128, but setPwm caps so its ok
    float right = R/127.0;
    driver.setPwm(driver.voltage_limit*left);
    driver2.setPwm(driver2.voltage_limit*right);
}

// Comm packet: LEFT (-128 to 127) | RIGHT (-128 to 127) | CHECKSUM (xor) 
void receivePacket(int bytes) {
    if (Wire.available() < 3) return;

    lastMsg = millis();

    // read bytes into 3 bytes
    int8_t packet[3];
    for (int i = 0; i < 3; i++) packet[i] = Wire.read();
    
    // check checksum, proceed or flag status but ignore
    // cast to unsigned int for XOR
    if (((uint8_t)packet[0] ^ (uint8_t)packet[1]) != (uint8_t)packet[2]) {
        digitalWrite(greenLED, LOW); digitalWrite(yellowLED, HIGH); digitalWrite(redLED, LOW);
    } else {
        digitalWrite(greenLED, HIGH); digitalWrite(yellowLED, LOW); digitalWrite(redLED, LOW);
        setDrive(packet[0],packet[1]);
    }
}

void setup() {
    pinMode(greenLED,OUTPUT);
    pinMode(yellowLED,OUTPUT);
    pinMode(redLED, OUTPUT);

    digitalWrite(greenLED, HIGH); // debug 
    delay(500);

    Wire.setSCL(SCLpin);
    Wire.setSDA(SDApin);
    Wire.begin(picoAddr);
    Wire.onReceive(receivePacket);

    digitalWrite(greenLED, LOW); // debug 
    delay(500);

    driver.voltage_power_supply = 18.0f;    driver2.voltage_power_supply = 18.0f;
    driver.voltage_limit = 18.0f;           driver2.voltage_limit = 18.0f;
    driver.pwm_frequency = 20000;           driver2.pwm_frequency = 20000;
    driver.init();                          driver2.init();
    driver.enable();                        driver2.enable();

    digitalWrite(greenLED, HIGH); // debug 
    delay(500);

    lastMsg = millis();

    digitalWrite(greenLED, LOW); // debug 
    delay(500);
}

void loop() {

    if (millis() - lastMsg > WATCHDOG_TIMEOUT) {
        setDrive(0,0);
        digitalWrite(greenLED, LOW); digitalWrite(yellowLED, LOW); digitalWrite(redLED, HIGH);
    }

    delay(delayMs);
}