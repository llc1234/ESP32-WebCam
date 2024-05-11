#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#include <uri/UriBraces.h>


#define WIFI_SSID ""
#define WIFI_PASS ""

esp32cam::Resolution initialResolution;

WebServer server(80);

long width = 800;
long height = 600;


void LiveStream() {
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
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(2000);

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
}


void loop() {
  server.handleClient();
}
