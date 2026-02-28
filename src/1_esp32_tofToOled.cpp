#include <Arduino.h>
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

const uint8_t SDApin = 21, SCLpin = 22, testPin = 15, oledAddr = 0x3C, picoAddr = 0x08;
const uint8_t SCREEN_WIDTH = 128, SCREEN_HEIGHT = 32;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void displayNum(uint16_t num) {
  String text = (String)num + " mm";

  display.clearDisplay();
  display.setCursor(0,0);
  display.print(text);
  display.display();
}

void setup() {
  Wire.begin(SDApin, SCLpin);
  
  if (display.begin(SSD1306_SWITCHCAPVCC, oledAddr)) {
    pinMode(testPin, OUTPUT);
    digitalWrite(testPin, HIGH);
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextWrap(true);

  display.clearDisplay();
  display.display();
}

void loop() {
  Wire.requestFrom(picoAddr, 2);

  if (Wire.available() == 2) {
    uint8_t buf[2];
    Wire.readBytes(buf, 2);
    uint16_t value = (buf[1] << 8) | buf[0];
    displayNum(value);
  }

  delay(100);
}