// ESP32の64ビットタイマを用いてワンショット割り込みを発生させる実験コード。
// 1000ミリ秒毎に10ミリ秒後に割り込みを発生させることを意図している。
// けれども1回目の割り込みは発生するものの、2回目以降の割り込みが発生しない。

#include <Arduino.h>

hw_timer_t *g_one_shot_timer = nullptr;
portMUX_TYPE g_one_shot_timer_mutex = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t g_one_shot_timer_counter = 0;

void IRAM_ATTR onOneShotTimer() {
  portENTER_CRITICAL_ISR(&g_one_shot_timer_mutex);
  g_one_shot_timer_counter++;
  portEXIT_CRITICAL_ISR(&g_one_shot_timer_mutex);
}

void setup() {
  Serial.begin(115200);

  constexpr uint8_t  timer_id = 0;
  constexpr uint16_t divider  = 80;  // 1us/step @80MHz
  constexpr bool     count_up = true;
  constexpr bool     edge     = true;
  g_one_shot_timer = timerBegin(timer_id, divider, count_up);
  timerAttachInterrupt(g_one_shot_timer, &onOneShotTimer, edge);
}

void loop() {
  // timerStop(g_one_shot_timer);
  // timerAlarmDisable(g_one_shot_timer);
  constexpr uint64_t interrupt_at = 10000;  // [step]
  constexpr bool     auto_reload  = false;
  timerAlarmWrite(g_one_shot_timer, interrupt_at, auto_reload);
  timerAlarmEnable(g_one_shot_timer);
  timerWrite(g_one_shot_timer, 0);
  // timerStart(g_one_shot_timer);

  portENTER_CRITICAL(&g_one_shot_timer_mutex);
  const uint32_t one_shot_timer_counter = g_one_shot_timer_counter;
  portEXIT_CRITICAL(&g_one_shot_timer_mutex);

  Serial.print("millis: ");
  Serial.println(millis());
  Serial.print("one_shot_timer_counter: ");
  Serial.println(one_shot_timer_counter);

  delay(1000);  // [ms]
}
