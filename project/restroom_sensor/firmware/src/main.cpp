// TODO: 設定値を`config.cpp`に保存するように変更する。
// TODO: 状態をMQTT経由で送信する処理を追加する。
// TODO: コマンドをMQTT経由で受信する処理を追加する。
// TODO: UDPの処理を削除する。

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include <NeoPixelBus.h>
#include <ArduinoJson.h>

#include "wifi_config.h"

// 光センサのピン番号
constexpr uint8_t kLightSensorPin = 33;  // ADC5

// 焦電センサのピン番号
constexpr uint8_t kPyroelectricSensorPin = 12;
// 焦電センサ関係の変数をロックするためのミューテックス
portMUX_TYPE g_pyroelectric_sensor_mutex = portMUX_INITIALIZER_UNLOCKED;
// 焦電センサの立ち下がりカウントが発生した回数
volatile uint32_t g_number_of_pyroelectric_sensor_interrupts = 0;

// LEDのピン番号
constexpr uint8_t  kLedPin   = 13;
// LEDの個数
constexpr uint16_t kLedCount =  1;
// LEDストリップ
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> g_led(kLedCount, kLedPin);

const IPAddress kMulticastAddress(224, 0, 0, 42);
constexpr uint16_t kMulticastPortDiscovery    = 10000;
constexpr uint16_t kMulticastPortNotification = 11000;
constexpr uint16_t kUnicastPortControl        = 12000;
String g_ip_address;
String g_host_name;
WiFiUDP g_udp_control;

void IRAM_ATTR handlePyroelectricSensorInterrupt() {
  // 現在時刻 [ms]
  const uint32_t current_time = millis();
  // 焦電センサの立ち下がりが発生した最終時刻 [ms]
  static volatile uint32_t s_last_handled_time = 0;
  // 焦電センサの立ち下がりの処理間隔 [ms]
  constexpr uint32_t kHandlingInterval = 1000 * 10;

  if ( current_time - s_last_handled_time > kHandlingInterval ) {
    portENTER_CRITICAL_ISR(&g_pyroelectric_sensor_mutex);
    g_number_of_pyroelectric_sensor_interrupts++;
    s_last_handled_time = current_time;
    portEXIT_CRITICAL_ISR(&g_pyroelectric_sensor_mutex);
  }
}

void setupIoPin() {
  pinMode(kLightSensorPin, INPUT);
  analogSetPinAttenuation(kLightSensorPin, ADC_6db);

  pinMode(kPyroelectricSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(kPyroelectricSensorPin), handlePyroelectricSensorInterrupt, FALLING);

  pinMode(kLedPin, OUTPUT);
}

void setupSerial() {
  Serial.begin(115200);
}

void setupLed() {
  g_led.Begin();
  g_led.Show();
}

void setLedColor(const RgbColor &color) {
  g_led.SetPixelColor(0, color);
  g_led.Show();
}

void setupOta() {
  setLedColor(RgbColor(255, 255, 255));
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    setLedColor(RgbColor(255, 0, 0));
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]() {
    setLedColor(RgbColor(0, 255, 0));
    Serial.println("Start updating...");
  });
  ArduinoOTA.onEnd([]() {
    setLedColor(RgbColor(0, 0, 255));
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    setLedColor(RgbColor(255, 0, 255));
    Serial.printf("Error[%u]: ", error);
    if      ( error == OTA_AUTH_ERROR    ) Serial.println("Auth Failed");
    else if ( error == OTA_BEGIN_ERROR   ) Serial.println("Begin Failed");
    else if ( error == OTA_CONNECT_ERROR ) Serial.println("Connect Failed");
    else if ( error == OTA_RECEIVE_ERROR ) Serial.println("Receive Failed");
    else if ( error == OTA_END_ERROR     ) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  setLedColor(RgbColor(0, 0, 0));
}

void setupUdpControlPort() {
  g_udp_control.begin(kUnicastPortControl);
}

void updateIpAddress() {
  g_ip_address = WiFi.localIP().toString();
  g_host_name  = ArduinoOTA.getHostname() + ".local";
}

void setup() {
  setupIoPin();
  setupSerial();
  setupLed();
  setupOta();
  setupUdpControlPort();

  updateIpAddress();
  Serial.print("g_ip_address: ");
  Serial.println(g_ip_address);
  Serial.print("g_host_name: ");
  Serial.println(g_host_name);
}

void handleDiscoveryMessage(const uint32_t current_time) {
  // 最後に電文を送信した時刻 [ms]
  static uint32_t s_last_sent_message_time = 0;
  // 定期送信間隔 [ms]
  constexpr uint32_t SendingPeriodicalInterval = 1000 * 60;

  // 電文を送信する必要があるか？
  bool needs_send = false;

  // まだ1度も送信したことがない？（起動直後に送信する）
  needs_send = needs_send || (s_last_sent_message_time == 0);
  // 最後の送信から一定時間以上が経過している？（定期的に送信する）
  needs_send = needs_send || (current_time - s_last_sent_message_time >= SendingPeriodicalInterval);

  if ( needs_send ) {
    updateIpAddress();

    StaticJsonBuffer<256> json_buffer;
    JsonObject &root = json_buffer.createObject();
    root["MessageType"]         = "DISCOVERY";
    root["HostName"]            = g_host_name;
    root["NotificationPort"]    = kMulticastPortNotification;
    root["NotificationAddress"] = kMulticastAddress.toString();
    root["ControlPort"]         = kUnicastPortControl;
    root["ControlAddress"]      = g_ip_address;

    WiFiUDP udp;
    udp.beginPacket(kMulticastAddress, kMulticastPortDiscovery);
    root.printTo(udp);
    udp.endPacket();

    s_last_sent_message_time = current_time;
  }
}

void handleNotificationMessage(const uint32_t current_time) {
  // 最後に電文を送信した時刻 [ms]
  static uint32_t s_last_sent_message_time = 0;
  // 最後に送信した光センサ値
  static uint16_t last_light_sensor_value = 0;
  // 最後に送信した焦電センサ割り込み回数
  static uint32_t last_number_of_pyroelectric_sensor_interrupts = 0;
  // 定期送信間隔 [ms]
  constexpr uint32_t SendingPeriodicalInterval = 1000 * 10;
  // 最小送信間隔 [ms]
  constexpr uint32_t kSendingMinimumInterval = 1000 * 2;

  // 光センサ値を取得する
  const uint16_t light_sensor_value = analogRead(kLightSensorPin);

  // 焦電センサ割り込み回数を取得する
  portENTER_CRITICAL(&g_pyroelectric_sensor_mutex);
  const uint32_t number_of_pyroelectric_sensor_interrupts = g_number_of_pyroelectric_sensor_interrupts;
  portEXIT_CRITICAL(&g_pyroelectric_sensor_mutex);

  // 電文を送信する必要があるか？
  bool needs_send = false;

  // 光センサ値が閾値を超えて変化している？
  needs_send = needs_send || (abs((int32_t)light_sensor_value - (int32_t)last_light_sensor_value) > (int32_t)2048);
  // 焦電センサ割り込み回数が変化している？
  needs_send = needs_send || (number_of_pyroelectric_sensor_interrupts != last_number_of_pyroelectric_sensor_interrupts);
  // まだ1度も送信したことがない？（起動直後に送信する）
  needs_send = needs_send || (s_last_sent_message_time == 0);
  // 最後の送信から一定時間以上が経過している？（定期的に送信する）
  needs_send = needs_send || (current_time - s_last_sent_message_time >= SendingPeriodicalInterval);
  // 最後の送信から一定時間以上が経過している？（頻繁に送信し過ぎないようにする）
  needs_send = needs_send && (current_time - s_last_sent_message_time >= kSendingMinimumInterval);

  if ( needs_send ) {
    StaticJsonBuffer<256> json_buffer;
    JsonObject &root = json_buffer.createObject();
    root["MessageType"]                          = "NOTIFICATION";
    root["HostName"]                             = g_host_name;
    root["LocalTime"]                            = current_time;
    root["LightSensorValue"]                     = light_sensor_value;
    root["NumberOfPyroelectricSensorInterrupts"] = number_of_pyroelectric_sensor_interrupts;

    WiFiUDP udp;
    udp.beginPacket(kMulticastAddress, kMulticastPortNotification);
    root.printTo(udp);
    udp.endPacket();

    s_last_sent_message_time                      = current_time;
    last_light_sensor_value                       = light_sensor_value;
    last_number_of_pyroelectric_sensor_interrupts = number_of_pyroelectric_sensor_interrupts;
  }
}

void handleControlMessage() {
  const int32_t size = g_udp_control.parsePacket();
  if ( size > 0 ) {
    char buffer[256] = {};
    g_udp_control.read(buffer, sizeof(buffer));

    StaticJsonBuffer<256> json_buffer;
    const JsonObject &root = json_buffer.parseObject(buffer);
    if ( root.success() ) {
      const String command = root["Command"].as<String>();

      if ( command == "SET_LED" ) {
        const JsonObject &color = root["Color"];
        const uint8_t red   = color["Red"];
        const uint8_t green = color["Green"];
        const uint8_t blue  = color["Blue"];
        setLedColor(RgbColor(red, green, blue));
      }
    }
  }
}

void loop() {
  ArduinoOTA.handle();

  const uint32_t current_time = millis();  // [ms]

  // ディスカバリ電文を送信する（必要であれば）
  handleDiscoveryMessage(current_time);
  // 通知電文を送信する（必要であれば）
  handleNotificationMessage(current_time);
  // 制御電文を処理する（必要であれば）
  handleControlMessage();

  delay(100);  // [ms]
}
