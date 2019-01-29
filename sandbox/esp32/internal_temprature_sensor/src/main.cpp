
#include <Arduino.h>

extern "C" {
  uint8_t temprature_sens_read();
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  const uint8_t temprature = temprature_sens_read();
  Serial.print("Temprature: ");
  Serial.println(temprature);
  delay(500);  // [ms]
}
