
#include <Arduino.h>
#include <WiFi.h>

#include <PubSubClient.h>

#include "config.h"

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}

void loop() {
}
