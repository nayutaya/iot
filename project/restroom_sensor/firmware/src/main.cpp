// TODO: コマンドをMQTT経由で受信する処理を追加する。
// TODO: UDPの処理を削除する。

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

extern const char    *kWifiSsid;
extern const char    *kWifiPassword;
extern const char    *kMqttServerAddress;
extern const uint16_t kMqttServerPort;
extern const char    *kMqttNotificationTopic;
extern const char    *kMqttControlTopic;

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

WiFiClient g_wifi_client;
PubSubClient g_pub_sub_client(g_wifi_client);
String g_ip_address;
String g_host_name;

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
  Serial.print(kWifiSsid);
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPassword);
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

  g_ip_address = WiFi.localIP().toString();
  g_host_name  = ArduinoOTA.getHostname() + ".local";
  Serial.print("g_ip_address: ");
  Serial.println(g_ip_address);
  Serial.print("g_host_name: ");
  Serial.println(g_host_name);

  setLedColor(RgbColor(0, 0, 0));
}

void setupMqtt() {
  g_pub_sub_client.setServer(kMqttServerAddress, kMqttServerPort);
  g_pub_sub_client.setCallback([](const char *topic, const byte *payload, const uint32_t length) {
    if ( String(topic).equals(kMqttControlTopic) ) {
      char buffer[256] = {};
      memcpy(buffer, payload, length);
      StaticJsonBuffer<256> json_buffer;
      const JsonObject &root = json_buffer.parseObject(buffer);
      if ( root.success() ) {
        const String command = root["Command"].as<String>();
        if ( command.equals("SET_LED") ) {
          const JsonObject &color = root["Color"];
          const uint8_t red   = color["Red"];
          const uint8_t green = color["Green"];
          const uint8_t blue  = color["Blue"];
          setLedColor(RgbColor(red, green, blue));
        }
      }
    }
  });
}

void setup() {
  setupIoPin();
  setupSerial();
  setupLed();
  setupOta();
  setupMqtt();
}

void handleOta() {
  ArduinoOTA.handle();
}

void handleMqtt() {
  if ( !g_pub_sub_client.connected() ) {
    if ( g_pub_sub_client.connect(g_host_name.c_str()) ) {
      g_pub_sub_client.subscribe(kMqttControlTopic);
    }
  }
  g_pub_sub_client.loop();
}

void handleNotificationMessage() {
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

  // 現在時刻 [ms]
  const uint32_t current_time = millis();
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
    root["IpAddress"]                            = g_ip_address;
    root["HostName"]                             = g_host_name;
    root["LocalTime"]                            = current_time;
    root["LightSensorValue"]                     = light_sensor_value;
    root["NumberOfPyroelectricSensorInterrupts"] = number_of_pyroelectric_sensor_interrupts;

    char buffer[256] = {0};
    root.printTo(buffer);
    Serial.println(buffer);
    g_pub_sub_client.publish(kMqttNotificationTopic, buffer);

    s_last_sent_message_time                      = current_time;
    last_light_sensor_value                       = light_sensor_value;
    last_number_of_pyroelectric_sensor_interrupts = number_of_pyroelectric_sensor_interrupts;
  }
}

void loop() {
  handleOta();
  handleMqtt();
  handleNotificationMessage();
  delay(100);  // [ms]
}
