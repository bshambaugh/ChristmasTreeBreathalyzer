#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266React.h>

#include "ButtonDuration.h"
#include "ButtonDurationService.h"

// Pins (edit for your board)
const int configButtonPin   = 0;  // BOOT
const int durationButtonPin = 8;  // BUTTON
const int ledPin            = 10; // LED

AsyncWebServer server(80);
ESP8266React esp8266React(&server);

// Register service using macros like official examples
SERVICE(ButtonDurationService, buttonDurationService, (&server, esp8266React.getSecurityManager()));

WiFiManager wifiManager;
bool shouldSaveConfig = false;

// WiFiManager callbacks
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

  Serial.println("Connected.");

  // Start framework
  esp8266React.begin();

  // Configure button service pins
  buttonDurationService.setupPins(durationButtonPin, ledPin);
  buttonDurationService.begin();

  // Start HTTP/WebSocket server
  server.begin();

  Serial.print("Ready! Open http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  esp8266React.loop();
  buttonDurationService.loop();
}
