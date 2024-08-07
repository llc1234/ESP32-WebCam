#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>



IPAddress local_IP(192, 168, 10, 160);  // Change this to your desired static IP
IPAddress gateway(192, 168, 10, 1);     // Set your network gateway
IPAddress subnet(255, 255, 255, 0);


// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// Set up a web server on port 80
ESP8266WebServer server(80);

void handleRoot() {
  String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Camera Search</title><style>body {display: flex;justify-content: center;align-items: center; height: 100vh;background-color: #f0f0f0;margin: 0;} .loader {display: flex;flex-direction: column;align-items: center;} .camera-icon {width: 50px;height: 50px;position: relative;border: 5px solid #333;border-radius: 5px;animation: shake 1.5s infinite;} .camera-icon:before, .camera-icon:after {content: '';position: absolute;background-color: #333;} .camera-icon:before {width: 30px;height: 10px;top: -15px;left: 50%;transform: translateX(-50%);} .camera-icon:after {width: 15px;height: 15px;border-radius: 50%;top: 50%;left: 50%;transform: translate(-50%, -50%);} @keyframes shake {0%, 100% { transform: translateX(0); } 25% { transform: translateX(-15px); } 50% { transform: translateX(15px); } 75% { transform: translateX(-15px); }} .loading-text {margin-top: 10px;font-size: 18px;color: #333;}</style></head><body><div class='loader'><div class='camera-icon'></div><div class='loading-text'>Searching for camera...</div></div><script>async function pingServer(url) {try {const response = await fetch(url, {method: 'HEAD',mode: 'no-cors'});if (response.ok || response.type === 'opaque') {setTimeout(() => {window.location.href = url;}, 2000);}} catch (error) {console.error('Server is not reachable:', error);}}const serverUrl = 'http://192.168.10.112:80/stream';setInterval(() => pingServer(serverUrl), 2000);</script></body></html>";
  server.send(200, "text/html", html);
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.print(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Set up server routing
  server.on("/", handleRoot);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
  checkWiFiConnection();
}
