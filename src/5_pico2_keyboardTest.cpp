// Implement standardized communication protocol + watchdog check

#include <Arduino.h>
#include <Wire.h>
#include "SimpleDCMotor.h"

const uint8_t IN1pin = 2, IN2pin = 3, ENpin = 0;
const uint8_t IN1pin1 = 4, IN2pin1 = 5, ENpin1 = 1;
const uint8_t SDApin = 20, SCLpin = 21, picoAddr = 0x08;
const uint8_t greenLED = 15, yellowLED = 17, redLED = 16;

unsigned long lastPing = 0;
const unsigned long WATCHDOG_TIMEOUT = 500; // ms, use millis()

#define CMD_DRIVE 0x01
#define CMD_STOP  0x02
#define CMD_BRAKE 0x03
#define CMD_PING  0x04

DCDriver2PWM driver = DCDriver2PWM(IN1pin, IN2pin, ENpin), driver2 = DCDriver2PWM(IN1pin1, IN2pin1, ENpin1);

void setDrive(float L, float R) {
    float left = L/255.0;
    float right = R/255.0;
    driver.setPwm(driver.voltage_limit*left);
    driver2.setPwm(driver2.voltage_limit*right);
}

// Comm packet: CMD (1 to 4) | LEFT (0 to 255) | RIGHT (0 to 255) | CHECKSUM (xor) 
void receivePacket(int bytes) {
    //Serial.println("packet received");

    // read bytes into 4 bytes
    uint8_t packet[4];
    for (int i = 0; i < 4; i++) packet[i] = Wire.read();

    // check checksum, proceed or flag status but ignore
    if ((packet[0] ^ packet[1] ^ packet[2]) != packet[3]) {
        digitalWrite(greenLED, LOW); digitalWrite(yellowLED, HIGH); digitalWrite(redLED, LOW);
        //Serial.println("checksum fail");
    } else {
        digitalWrite(greenLED, HIGH); digitalWrite(yellowLED, LOW); digitalWrite(redLED, LOW);
        switch(packet[0]) {
            case CMD_DRIVE:
                //Serial.println("drive");
                setDrive(packet[1], packet[2]);
                return;
            case CMD_STOP:
                //Serial.println("stop");
                setDrive(0,0);
                return;
            case CMD_BRAKE:
                //Serial.println("brake");
                setDrive(255,255);
                return;
            case CMD_PING:
                //Serial.println("ping");
                lastPing = millis();
                return;
        }
    }


}

void setup() {
    //Serial.begin(115200);
    pinMode(greenLED,OUTPUT);
    pinMode(yellowLED,OUTPUT);
    pinMode(redLED, OUTPUT);

    digitalWrite(greenLED, HIGH); // debug 
    delay(500);

    Wire.setSCL(SCLpin);
    Wire.setSDA(SDApin);
    Wire.begin(picoAddr);
    Wire.onReceive(receivePacket);
    //Serial.println("I2C configured");

    digitalWrite(greenLED, LOW); // debug 
    delay(500);

    driver.voltage_power_supply = 18.0f;    driver2.voltage_power_supply = 18.0f;
    driver.voltage_limit = 18.0f;           driver2.voltage_limit = 18.0f;
    driver.pwm_frequency = 20000;           driver2.pwm_frequency = 20000;
    driver.init();                          driver2.init();
    driver.enable();                        driver2.enable();
    //Serial.println("drivers configured");

    digitalWrite(greenLED, HIGH); // debug 
    delay(500);

    lastPing = millis();

    digitalWrite(greenLED, LOW); // debug 
    delay(500);
}

void loop() {

    if (millis() - lastPing > WATCHDOG_TIMEOUT) {
        setDrive(0,0);
        digitalWrite(greenLED, LOW); digitalWrite(yellowLED, LOW); digitalWrite(redLED, HIGH);
    }

    delay(10);
}