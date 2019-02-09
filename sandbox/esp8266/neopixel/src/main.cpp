
#include <Arduino.h>
#include <NeoPixelBus.h>

constexpr uint8_t  kLedPin   = 2;
constexpr uint16_t kLedCount = 8;

NeoPixelBus<NeoGrbFeature, NeoEsp8266AsyncUart1800KbpsMethod> strip(kLedCount, kLedPin);
RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor blue(0, 0, 255);
RgbColor white(255);
RgbColor black(0);
uint8_t count = 0;

void setup() {
  pinMode(kLedPin, OUTPUT);

  strip.Begin();
  strip.Show();
}

void loop() {
  for ( int i = 0; i < kLedCount; i++ ) {
    switch ( (count + i) % 5 ) {
      case 0: strip.SetPixelColor(i, red  ); break;
      case 1: strip.SetPixelColor(i, green); break;
      case 2: strip.SetPixelColor(i, blue ); break;
      case 3: strip.SetPixelColor(i, white); break;
      case 4: strip.SetPixelColor(i, black); break;
    }
  }
  strip.Show();

  count++;

  delay(1000);  // [ms]
}
