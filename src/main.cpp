#include <Arduino.h>
#include <WiFiManager.h>  // tzapu/WiFiManager (from DroneBot Workshop tutorial)
#include <ESP8266React.h>


// Pins: ESP32-C3 friendly (GPIO0 = BOOT button for config; GPIO8 = onboard LED often works for duration)
const int configButtonPin = 0;    // BOOT button for WiFi config trigger
const int durationButtonPin = 8;  // Duration button/LED (adjust if your wiring differs)
const int ledPin = 8;             // Onboard LED on many C3 boards

// Globals for duration service
unsigned long pressStartTime = 0;
bool wasPressed = false;

// WiFiManager globals
bool shouldSaveConfig = false;
WiFiManager wifiManager;

// Custom service for button duration (unchanged from your code)
class ButtonDurationService : public StatefulService<ButtonDurationService> {
public:
  unsigned long durationMs = 0;

  void setup() {
    pinMode(durationButtonPin, INPUT_PULLUP);  // Internal pull-up
    pinMode(ledPin, OUTPUT);
    pinMode(configButtonPin, INPUT_PULLUP);    // BOOT button
  }

  void loop() {
    // Duration logic
    bool buttonState = !digitalRead(durationButtonPin);  // LOW = pressed
    if (buttonState && !wasPressed) {
      pressStartTime = millis();
      digitalWrite(ledPin, HIGH);
      wasPressed = true;
    } else if (!buttonState && wasPressed) {
      this->durationMs = millis() - pressStartTime;
      digitalWrite(ledPin, LOW);
      wasPressed = false;
      this->publishState();  // Pushes to React UI via WebSocket
    }

    // Config button: Hold BOOT + press EN/RESET to enter portal (non-blocking check)
    if (digitalRead(configButtonPin) == LOW) {
      Serial.println("Config button pressed—starting portal");
      wifiManager.setConfigPortalTimeout(120);  // 2 min timeout
      if (!wifiManager.startConfigPortal("OnDemandAP")) {
        Serial.println("Failed to start portal and hit timeout");
        delay(3000);
        ESP.restart();
      }
      Serial.println("Config portal closed—connected!");
      while (digitalRead(configButtonPin) == LOW) delay(100);  // Debounce
    }

    // Save callback trigger
    if (shouldSaveConfig) {
      Serial.println("Configuration saved");
      shouldSaveConfig = false;
    }
  }

  // JSON read: Expose state to React UI
  static void read(const ButtonDurationService& state, JsonObject& root) {
    root["duration_ms"] = state.durationMs;
    root["status"] = state.durationMs > 0 ? F("Pressed for X ms") : F("Idle");
  }

  // JSON write: Optional reset from UI
  static StateUpdateResult update(JsonObject& root, ButtonDurationService& state) {
    if (root.containsKey("reset")) {
      state.durationMs = 0;
      return StateUpdateResult::CHANGED;
    }
    return StateUpdateResult::UNCHANGED;
  }
};

BUTTON_DURATION_SERVICE(buttonService);  // Registers the service

ESP8266React esp8266React(&server);


AsyncWebServer server(80);

// Save callback (from tutorial)
void saveConfigCallback() {
  Serial.println("Configuration saved");
  shouldSaveConfig = true;
}

// AP mode callback
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Entered Configuration Mode");
  Serial.print("Config SSID: "); Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.print("Config IP: "); Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);


  // Optional: Wipe settings for testing (comment out in production)
  // wifiManager.resetSettings();

  // Register callbacks
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);

  // Auto-connect: Starts "AutoConnectAP" AP (password: "password") if no saved creds
  bool res = wifiManager.autoConnect("AutoConnectAP", "password");
  if (!res) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  } else {
    Serial.println("Connected...yeey :)");
  }

  // Init service and framework (once connected)
  buttonService.begin();
  esp8266React.begin();
  server.begin();
  Serial.println("ESP32-C3 React + WiFiManager Ready! IP: " + WiFi.localIP().toString());
}

void loop() {
  esp8266React.loop();
  buttonService.loop();  // Your custom logic
}