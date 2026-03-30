// Implement standardized communication protocol + watchdog check
#include <Arduino.h>
#include <Wire.h>

// Comm packet: CMD (1 to 4) | LEFT (0 to 1) | RIGHT (0 to 1) | CHECKSUM (xor) 

#define CMD_DRIVE 0x01
#define CMD_STOP  0x02
#define CMD_BRAKE 0x03
#define CMD_PING  0x04

unsigned long lastPing;
String inputBuffer = "";
const uint8_t picoAddr = 0x08;

void send_packet(uint8_t cmd, uint8_t left = 0, uint8_t right = 0) {
    Wire.beginTransmission(picoAddr);
    Wire.write(cmd);
    Wire.write(left);
    Wire.write(right);
    Wire.write(cmd ^ left ^ 0);
    Wire.endTransmission();
}

void send_ping() {
    Wire.beginTransmission(picoAddr);
    Wire.write(CMD_PING);
    Wire.write(0); Wire.write(0);
    Wire.write(CMD_PING^0^0);
    Wire.endTransmission();
}

void handleInput(String input) {
    if (input == "stop") {
        send_packet(CMD_STOP);
    } else if (input == "brake") {
        send_packet(CMD_BRAKE);
    } else if (input.startsWith("drive")) {
        uint8_t l = (uint8_t)input.substring(6).toInt();
        uint8_t r = (uint8_t)input.substring(input.lastIndexOf(' ')).toInt();
        send_packet(CMD_DRIVE, l, r);
    }
}

void setup () {
    Wire.begin();
    Serial.begin(115200);
    lastPing = millis();
}

void loop() {
    if (millis() - lastPing > 200) {
        send_ping();
        lastPing = millis();
    }

    
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            inputBuffer.trim();
            handleInput(inputBuffer);
            inputBuffer = "";
        } else {
            inputBuffer += c;
        }
    }
}