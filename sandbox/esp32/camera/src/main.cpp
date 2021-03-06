#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_camera.h>
#include <WiFi.h>

#include <PubSubClient.h>

extern const char    *kWifiSsid;
extern const char    *kWifiPassword;
extern const char    *kMqttServerAddress;
extern const uint16_t kMqttServerPort;

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

void setupSerial() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
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

  esp_err_t err = esp_camera_init(&config);
  Serial.printf("esp_camera_init: 0x%x\n", err);

  // sensor_t *s = esp_camera_sensor_get();
  // s->set_framesize(s, FRAMESIZE_QVGA);
}

void setupMqtt() {
  g_pub_sub_client.setServer(kMqttServerAddress, kMqttServerPort);
}

void setup() {
  setupSerial();
  setupOta();
  setupCamera();
  setupMqtt();
}

void handleOta() {
  ArduinoOTA.handle();
}

void handleMqtt() {
  if ( !g_pub_sub_client.connected() ) {
    g_pub_sub_client.connect("esp32");
  }
  g_pub_sub_client.loop();
}

void loop() {
  handleOta();
  handleMqtt();

  camera_fb_t *fb = esp_camera_fb_get();
  if ( fb ) {
    Serial.printf("width: %d, height: %d, buf: 0x%x, len: %d\n", fb->width, fb->height, fb->buf, fb->len);
    g_pub_sub_client.publish("test", fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }

  delay(1000);  // [ms]
}
