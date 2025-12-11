#include <Arduino.h>
#include <WiFiManager.h>  // tzapu/WiFiManager
#include <ESP8266React.h>

// Pins (Beetle ESP32-C3: GPIO0 = BOOT button; onboard LED often on GPIO8)
const int configButtonPin   = 0;   // BOOT button for config portal
const int durationButtonPin = 8;   // Your duration button (with pull-up)
const int ledPin            = 10;   // Onboard LED (or external)

// Globals
unsigned long pressStartTime = 0;
bool wasPressed = false;

// WiFiManager
WiFiManager wifiManager;
bool shouldSaveConfig = false;

// Your custom service – state is the instance itself (common pattern)
class ButtonDurationService : public StatefulService<ButtonDurationService> {
 public:
  unsigned long durationMs = 0;  // Exposed state

  void setup() {
    pinMode(durationButtonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    pinMode(configButtonPin, INPUT_PULLUP);
  }

  void loop() {
    // --- Button duration logic ---
    bool buttonState = !digitalRead(durationButtonPin);  // LOW = pressed
    if (buttonState && !wasPressed) {
      pressStartTime = millis();
      digitalWrite(ledPin, HIGH);
      wasPressed = true;
    } else if (!buttonState && wasPressed) {
      // Update state via framework's update() → triggers WebSocket push
    this->update([&](ButtonDurationService& state) {
    state.durationMs = millis() - pressStartTime;
    return StateUpdateResult::CHANGED;
    }, "button_release");

      digitalWrite(ledPin, LOW);
      wasPressed = false;
    }

    // --- On-demand WiFi config portal (press BOOT button) ---
    if (digitalRead(configButtonPin) == LOW) {
      Serial.println("Config button pressed — starting portal");
      wifiManager.setConfigPortalTimeout(120);
      if (!wifiManager.startConfigPortal("OnDemandAP", "password")) {
        Serial.println("Portal failed or timed out");
        delay(3000);
        ESP.restart();
      }
      Serial.println("Portal closed — reconnected!");
      while (digitalRead(configButtonPin) == LOW) delay(100);  // Simple debounce
    }

    if (shouldSaveConfig) {
      Serial.println("Configuration saved");
      shouldSaveConfig = false;
    }
  }

  // JSON serialization for UI
  static void read(const ButtonDurationService& state, JsonObject& root) {
    root["duration_ms"] = state.durationMs;
    root["status"] = state.durationMs > 0 ? F("Last press: %lu ms") : F("Idle");
  }

  // Optional: Handle writes from UI (e.g., reset button)
  static StateUpdateResult update(JsonObject& root, ButtonDurationService& state) {
    if (root.containsKey("reset")) {
      state.durationMs = 0;
      return StateUpdateResult::CHANGED;
    }
    return StateUpdateResult::UNCHANGED;
  }
};

BUTTON_DURATION_SERVICE(buttonService);  // Registers the service

// Framework
AsyncWebServer server(80);
ESP8266React esp8266React(&server);

// Callbacks
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

  WiFi.mode(WIFI_STA);  // Required for ESP32 + WiFiManager

  // Callbacks
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);

  // Auto-connect (creates "AutoConnectAP" if no saved creds)
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("WiFi connect failed — restarting");
    delay(3000);
    ESP.restart();
  }
  Serial.println("Connected...yeey :)");

  // Init your service + framework
  buttonService.begin();
  esp8266React.begin();
  server.begin();

  Serial.print("Ready! Open http://");
  Serial.print(WiFi.localIP());
  Serial.println(" (login: admin/admin)");
}

void loop() {
  esp8266React.loop();     // Framework (WebSockets, etc.)
  buttonService.loop();  // Your button logic
}