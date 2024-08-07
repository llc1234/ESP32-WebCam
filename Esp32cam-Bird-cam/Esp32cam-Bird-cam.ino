#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#include <uri/UriBraces.h>
#include <esp_wifi.h>
#include <esp_sleep.h>


IPAddress local_IP(192, 168, 10, 112); // Change to your desired static IP
IPAddress gateway(192, 168, 10, 1);    // Change to your network gateway
IPAddress subnet(255, 255, 255, 0);



#define WIFI_SSID ""
#define WIFI_PASS ""

esp32cam::Resolution initialResolution;

WebServer server(80);

long width = 800;
long height = 600;

// Define how long to stay awake after serving a request
const long ACTIVE_PERIOD_MS = 60000;
unsigned long lastRequestTime = 0;

void LiveStream() {
  if (server.hasArg("w")) {
    width = server.arg("w").toInt();
  }

  if (server.hasArg("h")) {
    height = server.arg("h").toInt();
  }
    
  auto r = esp32cam::Camera.listResolutions().find(width, height);

  if (!esp32cam::Camera.changeResolution(r)) {
    Serial.printf("changeResolution(%ld,%ld) failure\n", width, height);
    server.send(500, "text/plain", "changeResolution error\n");
  }
    
  Serial.printf("changeResolution(%ld,%ld) success\n", width, height);
  
  Serial.println("MJPEG streaming begin");
  WiFiClient client = server.client();
  auto startTime = millis();
  int nFrames = esp32cam::Camera.streamMjpeg(client);
  auto duration = millis() - startTime;
  Serial.printf("MJPEG streaming end: %dfrm %0.2ffps\n", nFrames, 1000.0 * nFrames / duration);
  
  lastRequestTime = millis();
}

void handleCapture() {
  auto img = esp32cam::capture();
  if (img == nullptr) {
    server.send(500, "", "");
    return;
  }
  server.setContentLength(img->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  img->writeTo(client);

  lastRequestTime = millis();
}

void enterDeepSleep() {
  Serial.println("Entering sleep mode...");
  delay(100);
  esp_sleep_enable_timer_wakeup(1000000 * 60);
  // esp_deep_sleep_start();
  esp_light_sleep_start();
  Serial.println("timer wakeup mode...");
  lastRequestTime = millis();
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  delay(2000);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi failure %d\n", WiFi.status());
    delay(5000);
    ESP.restart();
  }
  Serial.println("WiFi connected");

  initialResolution = esp32cam::Resolution::find(width, height);

  esp32cam::Config cfg;
  cfg.setPins(esp32cam::pins::AiThinker);
  cfg.setResolution(initialResolution);
  cfg.setJpeg(80);

  bool ok = esp32cam::Camera.begin(cfg);
  if (!ok) {
    Serial.println("camera initialize failure");
    delay(5000);
    ESP.restart();
  }
  Serial.println("camera initialize success");

  Serial.println("camera starting");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  server.on("/stream", LiveStream);
  server.on("/capture", handleCapture);
  
  server.begin();

  lastRequestTime = millis();
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.disconnect();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  server.handleClient();

  if (millis() - lastRequestTime > ACTIVE_PERIOD_MS) {
    checkWiFiConnection();
    enterDeepSleep();
  }
}
