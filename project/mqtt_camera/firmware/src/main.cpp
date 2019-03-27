#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_camera.h>
#include <WiFi.h>

#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "config.h"

// REF: https://github.com/SeeedDocument/forum_doc/blob/master/reg/ESP32_CAM_V1.6.pdf
constexpr int kCameraPin_PWDN   =  32;
constexpr int kCameraPin_RESET  =  -1;  // NC
constexpr int kCameraPin_XCLK   =   0;
constexpr int kCameraPin_SIOD   =  26;
constexpr int kCameraPin_SIOC   =  27;
constexpr int kCameraPin_Y9     =  35;
constexpr int kCameraPin_Y8     =  34;
constexpr int kCameraPin_Y7     =  39;
constexpr int kCameraPin_Y6     =  36;
constexpr int kCameraPin_Y5     =  21;
constexpr int kCameraPin_Y4     =  19;
constexpr int kCameraPin_Y3     =  18;
constexpr int kCameraPin_Y2     =   5;
constexpr int kCameraPin_VSYNC  =  25;
constexpr int kCameraPin_HREF   =  23;
constexpr int kCameraPin_PCLK   =  22;

WiFiClient g_wifi_client;
PubSubClient g_pub_sub_client(g_wifi_client);

bool g_is_capture_requested = false;

void setupSerial() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
}

void setupWifi() {
  Serial.printf("[WiFi] Connecting to %s...\n", kWifiSsid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPassword);
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    Serial.println("[WiFi] Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}

void setupOta() {
  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Start updating...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]: ", error);
    if      ( error == OTA_AUTH_ERROR    ) Serial.println("Auth Failed");
    else if ( error == OTA_BEGIN_ERROR   ) Serial.println("Begin Failed");
    else if ( error == OTA_CONNECT_ERROR ) Serial.println("Connect Failed");
    else if ( error == OTA_RECEIVE_ERROR ) Serial.println("Receive Failed");
    else if ( error == OTA_END_ERROR     ) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setupCamera() {
  const camera_config_t config = {
    .pin_pwdn     = kCameraPin_PWDN,
    .pin_reset    = kCameraPin_RESET,
    .pin_xclk     = kCameraPin_XCLK,
    .pin_sscb_sda = kCameraPin_SIOD,
    .pin_sscb_scl = kCameraPin_SIOC,
    .pin_d7       = kCameraPin_Y9,
    .pin_d6       = kCameraPin_Y8,
    .pin_d5       = kCameraPin_Y7,
    .pin_d4       = kCameraPin_Y6,
    .pin_d3       = kCameraPin_Y5,
    .pin_d2       = kCameraPin_Y4,
    .pin_d1       = kCameraPin_Y3,
    .pin_d0       = kCameraPin_Y2,
    .pin_vsync    = kCameraPin_VSYNC,
    .pin_href     = kCameraPin_HREF,
    .pin_pclk     = kCameraPin_PCLK,
    .xclk_freq_hz = 20000000,
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size   = FRAMESIZE_SVGA,
    .jpeg_quality = 10,
    .fb_count     = 1,
  };

  const esp_err_t err = esp_camera_init(&config);
  Serial.printf("[Camera] esp_camera_init: 0x%08x\n", err);

  // sensor_t *s = esp_camera_sensor_get();
  // s->set_framesize(s, FRAMESIZE_QVGA);
}

void setupMqtt() {
  g_pub_sub_client.setServer(kMqttServerAddress, kMqttServerPort);
  g_pub_sub_client.setCallback([](const char *topic, const byte *payload, const uint32_t length) {
    Serial.printf("[MQTT] Callback(topic: %s, payload: 0x%08x, length: %d)\n", topic, (unsigned int)payload, length);
    if ( String(topic).equals(kMqttRequestTopic) ) {
      constexpr size_t buffer_size = 256;
      char buffer[buffer_size] = {};
      if ( length > buffer_size ) return;
      memcpy(buffer, payload, length);
      StaticJsonDocument<256> json_doc;
      const auto error = deserializeJson(json_doc, buffer);
      if ( error ) return;
      // TODO: 撮影要求電文を解析する。
      Serial.println("[MQTT] command: capture");
      g_is_capture_requested = true;
    }
  });
}

void setup() {
  setupSerial();
  setupWifi();
  setupOta();
  setupCamera();
  setupMqtt();

  Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WiFi] Host Name: %s\n", ArduinoOTA.getHostname().c_str());
}

void handleOta() {
  ArduinoOTA.handle();
}

void handleMqtt() {
  if ( !g_pub_sub_client.connected() ) {
    if ( g_pub_sub_client.connect(ArduinoOTA.getHostname().c_str()) ) {
      g_pub_sub_client.subscribe(kMqttRequestTopic);
    }
  }
  g_pub_sub_client.loop();
}

void loop() {
  handleOta();
  handleMqtt();

  if ( g_is_capture_requested ) {
    g_is_capture_requested = false;

    camera_fb_t *fb = esp_camera_fb_get();
    if ( fb ) {
      Serial.printf("[Camera] width: %d, height: %d, buffer: 0x%08x, len: %d\n", fb->width, fb->height, (unsigned int)fb->buf, fb->len);
      g_pub_sub_client.publish(kMqttResponseTopic, fb->buf, fb->len);
      esp_camera_fb_return(fb);
    }
  }

  delay(100);  // [ms]
}
