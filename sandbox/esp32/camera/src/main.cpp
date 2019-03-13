#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

extern const char *kWifiSsid;
extern const char *kWifiPassword;

// constexpr uint8_t kLedPin = 2;

void setupSerial() {
  Serial.begin(115200);
}

void setupOta() {
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

  Serial.print("WiFi.localIP: ");
  Serial.println(WiFi.localIP().toString());
  Serial.print("ArduinoOTA.getHostname: ");
  Serial.println(ArduinoOTA.getHostname());
}

void setup() {
  setupSerial();
  setupOta();
}

void handleOta() {
  ArduinoOTA.handle();
}

void loop() {
  handleOta();

  Serial.println("Hello world!");

  // digitalWrite(kLedPin, HIGH);
  // delay(1000);
  // digitalWrite(kLedPin, LOW);

  delay(1000);  // [ms]
}
