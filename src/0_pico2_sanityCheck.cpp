#include <Arduino.h>

unsigned long lastTime = 0;
bool on = true;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    if (millis() - lastTime > 1000) {
        on = !on;
        digitalWrite(LED_BUILTIN, on);
        lastTime = millis();
    }
}