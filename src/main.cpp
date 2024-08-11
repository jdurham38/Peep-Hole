#include "esp_camera.h"
#include <WiFi.h>
#include "camera_index.h"
#include "camera_pins.h"
#include <WebServer.h>
#include <Ticker.h>

const char* ssid = "";
const char* password = "";

const int pirPin = 13; // GPIO pin connected to PIR sensor
volatile bool motionDetected = false;
volatile bool streaming = false;
Ticker stopStreamTicker;
unsigned long lastMotionTime = 0;

WebServer server(80);

void IRAM_ATTR detectMotion() {
  motionDetected = true;
  lastMotionTime = millis();
}

void stopStream() {
  if (millis() - lastMotionTime >= 5000) { // Check if 5 seconds have passed since last motion
    streaming = false;
    stopStreamTicker.detach();
    Serial.println("Streaming stopped due to inactivity.");
  }
}

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void handleStream() {
  if (!streaming) {
    server.send(403, "text/plain", "Streaming not allowed");
    return;
  }

  WiFiClient client = server.client();
  String boundary = "frame";
  String response = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: multipart/x-mixed-replace; boundary=" + boundary + "\r\n"
                    "Connection: close\r\n\r\n";
  server.sendContent(response);

  while (client.connected() && streaming) {
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

    delay(10); // Adjust or remove this delay for better performance
  }
  Serial.println("Client disconnected from stream.");
}

void handleEvents() {
  WiFiClient client = server.client();
  if (!client) {
    Serial.println("No client for events.");
    return;
  }

  String response = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/event-stream\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n";
  client.print(response);
  Serial.println("Client connected for events.");

  while (client.connected()) {
    if (streaming) {
      String event = "event: streamstart\n";
      event += "data: Stream started\n\n";
      client.print(event);
      delay(1000); // Sending updates every second, adjust as necessary
    }
  }
  Serial.println("Client disconnected from events.");
}


void startCameraServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/events", HTTP_GET, handleEvents);
  server.onNotFound(handleNotFound);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(pirPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pirPin), detectMotion, RISING);

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

  config.frame_size = FRAMESIZE_CIF; // Reduce frame size for lower latency
  config.jpeg_quality = 10;          // Adjust JPEG quality to balance quality and speed
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  startCameraServer();
  Serial.printf("Camera Ready! Use 'http://%s' to connect\n", WiFi.localIP().toString().c_str());
}

void loop() {
  if (motionDetected) {
    motionDetected = false;
    if (!streaming) {
      streaming = true;
      Serial.println("Motion detected! Starting stream...");
      stopStreamTicker.attach(5, stopStream); // Schedule the stopStream function to run every 5 seconds
    }
  }

  server.handleClient();
}

