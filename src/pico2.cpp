#include <Arduino.h>

void setup() {
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
}

void loop() {
    analogWrite(2, 128);
    analogWrite(3, 255);
    analogWrite(4, 200);
}