#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_camera.h>
#include <WiFi.h>

#include <PubSubClient.h>

extern const char    *kWifiSsid;
extern const char    *kWifiPassword;
extern const char    *kMqttServerAddress;
extern const uint16_t kMqttServerPort;

// constexpr uint8_t kLedPin = 2;

constexpr int8_t PWDN_GPIO_NUM   =  32;
constexpr int8_t RESET_GPIO_NUM  =  -1;
constexpr int8_t XCLK_GPIO_NUM   =   0;
constexpr int8_t SIOD_GPIO_NUM   =  26;
constexpr int8_t SIOC_GPIO_NUM   =  27;

constexpr int8_t Y9_GPIO_NUM     =  35;
constexpr int8_t Y8_GPIO_NUM     =  34;
constexpr int8_t Y7_GPIO_NUM     =  39;
constexpr int8_t Y6_GPIO_NUM     =  36;
constexpr int8_t Y5_GPIO_NUM     =  21;
constexpr int8_t Y4_GPIO_NUM     =  19;
constexpr int8_t Y3_GPIO_NUM     =  18;
constexpr int8_t Y2_GPIO_NUM     =   5;
constexpr int8_t VSYNC_GPIO_NUM  =  25;
constexpr int8_t HREF_GPIO_NUM   =  23;
constexpr int8_t PCLK_GPIO_NUM   =  22;

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

void setup() {
  setupSerial();
  setupOta();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  Serial.printf("esp_camera_init: 0x%x\n", err);

  // sensor_t *s = esp_camera_sensor_get();
  // s->set_framesize(s, FRAMESIZE_QVGA);

  g_pub_sub_client.setServer(kMqttServerAddress, kMqttServerPort);
}

void handleOta() {
  ArduinoOTA.handle();
}

void loop() {
  handleOta();

  if ( !g_pub_sub_client.connected() ) {
    g_pub_sub_client.connect("esp32");
  }
  g_pub_sub_client.loop();

  // Serial.println("Hello world!");
  // digitalWrite(kLedPin, HIGH);
  // delay(1000);
  // digitalWrite(kLedPin, LOW);

  camera_fb_t *fb = esp_camera_fb_get();
  if ( fb ) {
    Serial.printf("width: %d, height: %d, buf: 0x%x, len: %d\n", fb->width, fb->height, fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }

  const uint32_t current_time = millis();
  static uint32_t last_time = 0;

  if ( current_time - last_time >= 1000 ) {
    g_pub_sub_client.publish("test", "hello");
    Serial.println("publish");
    last_time = current_time;
  }

  delay(1000);  // [ms]
}
