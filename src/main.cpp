#include "esp_camera.h"
#include <WiFi.h>
#include "camera_index.h"
#include "camera_pins.h"
#include <WebServer.h>
#include <esp_task_wdt.h>

const char* ssid = "**********";
const char* password = "**********";

WebServer server(80);
unsigned long lastHeartbeat = 0;
const int WDT_TIMEOUT = 10;  // seconds

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

const int FPS = 10; // Target 10 frames per second
const int frame_delay = 1000 / FPS;

void handleStream() {
  WiFiClient client = server.client();
  String boundary = "frame";
  String response = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: multipart/x-mixed-replace; boundary=" + boundary + "\r\n"
                    "Connection: close\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }

    client.printf("--%s\r\n", boundary.c_str());
    client.printf("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.printf("\r\n");

    esp_camera_fb_return(fb);

    delay(frame_delay); // Delay between frames to control FPS
  }
  client.stop();
  Serial.println("Client disconnected from stream.");
}


void startCameraServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.onNotFound(handleNotFound);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Improve stream quality
  config.frame_size = FRAMESIZE_QVGA; // Use higher resolution
  config.jpeg_quality = 12;          // Lower quality value for higher quality image
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();  // Restart the ESP32 if the camera fails to initialize
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  startCameraServer();
  Serial.printf("Camera Ready! Use 'http://%s' to connect\n", WiFi.localIP().toString().c_str());

  // Initialize watchdog timer
  esp_task_wdt_init(WDT_TIMEOUT, true);  // Enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);  // Add current thread to WDT
}

void loop() {
  server.handleClient();

  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Reconnecting to WiFi...");
    }
    Serial.println("Reconnected to WiFi");
  }

  // Heartbeat to check server responsiveness
  if (millis() - lastHeartbeat > 5000) {
    Serial.println("Heartbeat: Server is running.");
    lastHeartbeat = millis();
  }

  // Reset watchdog timer
  esp_task_wdt_reset();
}
