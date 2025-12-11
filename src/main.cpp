#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include <ESP8266React.h>                     // original, untouched framework
#include "ButtonDurationService.h"

// Pins (adjust if you moved them on the Beetle ESP32-C3)
const int configButtonPin   = 9;    // GPIO9 is usually the BOOT button on C3 boards
const int durationButtonPin = 8;
const int ledPin            = 10;

AsyncWebServer server(80);
ESP8266React   esp8266React(&server);

// Your service — now using the original framework classes (HttpEndpoint, etc.)
ButtonDurationService buttonDurationService(&server, esp8266React.getSecurityManager());

WiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Optional: reset saved settings for testing
  // wifiManager.resetSettings();

  wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("Entered Config Mode");
    Serial.print("AP SSID: ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  });

  if (!wifiManager.autoConnect("ButtonDuration-AP", "12345678")) {
    Serial.println("Failed to connect — restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the whole React framework (security, settings service, etc.)
  esp8266React.begin();

  // Register your custom service AFTER esp8266React.begin()
  buttonDurationService.begin();

  // Setup the physical pins
  buttonDurationService.setupPins(durationButtonPin, ledPin);

  // Start the async server (framework already added its routes, we just start listening)
  server.begin();

  Serial.println("HTTP server started — open the IP in your browser");
}

void loop() {
  // The framework and your service do all the work
  esp8266React.loop();
  buttonDurationService.loop();
}