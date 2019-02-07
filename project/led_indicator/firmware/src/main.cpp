
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include <ArduinoJson.h>
#include <NeoPixelBus.h>
#include <PubSubClient.h>

extern const char    *kWifiSsid;
extern const char    *kWifiPassword;
extern const char    *kMqttServerAddress;
extern const uint16_t kMqttServerPort;
extern const char    *kMqttControlTopic;

// LEDのピン番号
constexpr uint8_t  kLedPin   = 2;
// LEDの個数
constexpr uint16_t kLedCount = 8;
// LEDストリップ
NeoPixelBus<NeoGrbFeature, NeoEsp8266AsyncUart1800KbpsMethod> g_led(kLedCount, kLedPin);

WiFiClient g_wifi_client;
PubSubClient g_pub_sub_client(g_wifi_client);
String g_ip_address;
String g_host_name;

void setupIoPin() {
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
  for ( int i = 0; i < kLedCount; i++ ) {
    g_led.SetPixelColor(i, color);
  }
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

void loop() {
  handleOta();
  handleMqtt();
  delay(100);  // [ms]
}
