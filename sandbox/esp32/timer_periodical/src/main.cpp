// ESP32の64ビットタイマを用いて連続的に割り込みを発生させる実験コード。
// 10ミリ秒毎に割り込みが発生するため、1秒毎に[periodical_timer_counter]が100増加する。

#include <Arduino.h>

hw_timer_t *g_periodical_timer = nullptr;
portMUX_TYPE g_periodical_timer_mutex = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t g_periodical_timer_counter = 0;

void IRAM_ATTR onPeriodicalTimer() {
  portENTER_CRITICAL_ISR(&g_periodical_timer_mutex);
  g_periodical_timer_counter++;
  portEXIT_CRITICAL_ISR(&g_periodical_timer_mutex);
}

void setup() {
  Serial.begin(115200);

  constexpr uint8_t  timer_id     = 0;
  constexpr uint16_t divider      = 80;  // 1us/step @80MHz
  constexpr bool     count_up     = true;
  constexpr bool     edge         = true;
  constexpr uint64_t interrupt_at = 10000;  // [step]
  constexpr bool     auto_reload  = true;
  g_periodical_timer = timerBegin(timer_id, divider, count_up);
  timerAttachInterrupt(g_periodical_timer, &onPeriodicalTimer, edge);
  timerAlarmWrite(g_periodical_timer, interrupt_at, auto_reload);
  timerAlarmEnable(g_periodical_timer);
}

void loop() {
  portENTER_CRITICAL(&g_periodical_timer_mutex);
  const uint32_t periodical_timer_counter = g_periodical_timer_counter;
  portEXIT_CRITICAL(&g_periodical_timer_mutex);

  Serial.print("millis: ");
  Serial.println(millis());
  Serial.print("periodical_timer_counter: ");
  Serial.println(periodical_timer_counter);

  delay(1000);  // [ms]
}
