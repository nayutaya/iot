
#include <Arduino.h>

constexpr uint8_t kPyroelectricSensorPin = 33;
constexpr uint32_t kPyroelectricSensorInterruptInterval = 5000;
portMUX_TYPE g_pyroelectric_sensor_mutex = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t g_number_of_pyroelectric_sensor_interrupts = 0;
volatile uint32_t g_last_pyroelectric_sensor_interrupt_handled_time = 0;

void IRAM_ATTR handlePyroelectricSensorInterrupt() {
  const uint32_t current_time = millis();

  if ( current_time - g_last_pyroelectric_sensor_interrupt_handled_time > kPyroelectricSensorInterruptInterval ) {
    portENTER_CRITICAL_ISR(&g_pyroelectric_sensor_mutex);
    g_number_of_pyroelectric_sensor_interrupts++;
    g_last_pyroelectric_sensor_interrupt_handled_time = current_time;
    portEXIT_CRITICAL_ISR(&g_pyroelectric_sensor_mutex);
  }
}

void setup() {
  pinMode(kPyroelectricSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(kPyroelectricSensorPin), handlePyroelectricSensorInterrupt, FALLING);
  Serial.begin(115200);
}

void loop() {
  const bool isHumanDetected = (digitalRead(kPyroelectricSensorPin) == LOW);
  Serial.print("isHumanDetected: ");
  Serial.println(isHumanDetected);
  Serial.print("g_number_of_pyroelectric_sensor_interrupts: ");
  Serial.println(g_number_of_pyroelectric_sensor_interrupts);
  delay(200);  // [ms]
}
