
#include <Arduino.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "wifi_config.h"

WiFiUDP udp;
uint8_t buffer[16] = {};

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]() {
    Serial.println("Start updating...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if      ( error == OTA_AUTH_ERROR    ) Serial.println("Auth Failed");
    else if ( error == OTA_BEGIN_ERROR   ) Serial.println("Begin Failed");
    else if ( error == OTA_CONNECT_ERROR ) Serial.println("Connect Failed");
    else if ( error == OTA_RECEIVE_ERROR ) Serial.println("Receive Failed");
    else if ( error == OTA_END_ERROR     ) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.println(ArduinoOTA.getHostname());

  udp.begin(3000);
}

void loop() {
  ArduinoOTA.handle();

  Serial.println("loop");

  int32_t size = udp.parsePacket();

  if ( size > 0 ) {
    Serial.print("size: ");
    Serial.println(size);

    udp.read(buffer, sizeof(buffer));

    Serial.print("buffer: ");
    for ( int32_t i = 0; i < size; i++ ) {
      Serial.print(buffer[i]);
    }
    Serial.println();
  }

  delay(1000);
}
