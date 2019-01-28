
#include <Arduino.h>
#include <WiFi.h>

#include <PubSubClient.h>

extern const char    *kWifiSsid;
extern const char    *kWifiPassword;
extern const char    *kMqttServerAddress;
extern const uint16_t kMqttServerPort;

WiFiClient g_wifi_client;
PubSubClient g_pub_sub_client(g_wifi_client);

void callback(const char *topic, const byte *payload, const uint16_t length) {
  Serial.print("callback");
}

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.print(kWifiSsid);
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPassword);
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Connected.");

  Serial.print("WiFi.localIP: ");
  Serial.println(WiFi.localIP());

  g_pub_sub_client.setServer(kMqttServerAddress, kMqttServerPort);
  g_pub_sub_client.setCallback(callback);
}

void loop() {
  g_pub_sub_client.loop();

  const uint32_t current_time = millis();
  static uint32_t last_time = 0;

  if ( current_time - last_time >= 1000 ) {
    g_pub_sub_client.publish("test", "hello");
    Serial.println("publish");
    last_time = current_time;
  }
}
