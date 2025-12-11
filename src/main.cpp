#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266React.h>

#include "ButtonDuration.h"
#include "ButtonDurationService.h"
#//include "disable_nghttp2.h"

// Pins
const int configButtonPin   = 0;   // BOOT
const int durationButtonPin = 8;   // BUTTON
const int ledPin            = 10;  // LED

AsyncWebServer server(80);
ESP8266React esp8266React(&server);

// === MANUAL SERVICE INSTANTIATION (no SERVICE macro) ===
ButtonDurationService buttonDurationService(&server, esp8266React.getSecurityManager());

WiFiManager wifiManager;
bool shouldSaveConfig = false;

void saveConfigCallback() {
  Serial.println("WiFiManager: Configuration saved");
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager* myWiFiManager) {
  Serial.println("WiFiManager: Entered config mode");
  Serial.print("AP SSID: "); Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("WiFi connect failed — restarting");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Connected to WiFi");

  // Start the React framework (registers security, MQTT, etc.)
  esp8266React.begin();

  // === IMPORTANT: Register your service endpoints AFTER esp8266React.begin() ===
  buttonDurationService.begin();

  // Configure pins
  buttonDurationService.setupPins(durationButtonPin, ledPin);

  // Start web server
  server.begin();

  Serial.print("Ready! Open http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  esp8266React.loop();
  buttonDurationService.loop();
}