
#include <Arduino.h>

constexpr uint8_t kLedPin = 2;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  pinMode(kLedPin, OUTPUT);
}

void loop() {
  Serial.println("Hello world!");

  digitalWrite(kLedPin, HIGH);
  delay(1000);
  digitalWrite(kLedPin, LOW);
  delay(1000);
}
