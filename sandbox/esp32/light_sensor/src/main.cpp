
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
}

void loop() {
  const auto value = analogRead(34);
  Serial.println(value);
  delay(1000);  // [ms]
}
