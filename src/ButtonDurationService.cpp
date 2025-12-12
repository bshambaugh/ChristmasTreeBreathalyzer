#include "ButtonDurationService.h"

ButtonDurationService::ButtonDurationService(AsyncWebServer* server,
                                             SecurityManager* securityManager) :
  _httpEndpoint(ButtonDuration::read,
                ButtonDuration::update,
                this,
                server,
                BUTTON_DURATION_ENDPOINT_PATH,
                securityManager,
                AuthenticationPredicates::IS_AUTHENTICATED),
  _ws(ButtonDuration::read,
      ButtonDuration::update,
      this,
      server,
      BUTTON_DURATION_WS_PATH,
      securityManager,
      AuthenticationPredicates::IS_AUTHENTICATED)
{
  // Member initialisation is enough — everything else happens in begin()
}

void ButtonDurationService::setupPins(int buttonPin, int ledPin) {
  _buttonPin = buttonPin;
  _ledPin = ledPin;

  pinMode(_buttonPin, INPUT_PULLUP);
  pinMode(_ledPin, OUTPUT);

  digitalWrite(_ledPin, LOW);  // make sure LED is off at start

  Serial.println();
  Serial.println(F("ButtonDurationService: Pins configured"));
  Serial.printf("   Button pin : GPIO%d (active LOW with pull-up)\n", _buttonPin);
  Serial.printf("   LED pin    : GPIO%d\n", _ledPin);
  Serial.println(F("   Ready — press the button!"));
  Serial.println();
}

void ButtonDurationService::begin() {
  // Reset state to 0 on boot
  update([](ButtonDuration& state) {
    state.durationMs = 0;
    return StateUpdateResult::CHANGED;
  }, "init");

  Serial.println(F("ButtonDurationService: Service started, duration reset to 0"));
}

void ButtonDurationService::loop() {
  if (_buttonPin < 0) return;  // pins not configured yet

  bool currentlyPressed = !digitalRead(_buttonPin);  // active-low with pull-up

  // ── Button just pressed ─────────────────────
  if (currentlyPressed && !wasPressed) {
    pressStartTime = millis();
    digitalWrite(_ledPin, HIGH);
    wasPressed = true;

    Serial.println();
    Serial.println(F("BUTTON PRESSED"));
    Serial.print(F("   Start time : "));
    Serial.println(pressStartTime);
  }

  // ── Button just released ─────────────────────
  else if (!currentlyPressed && wasPressed) {
    unsigned long duration = millis() - pressStartTime;

    // Update the state that is sent to the web UI and WebSocket clients
    this->update([duration](ButtonDuration& state) {
      state.durationMs = duration;
      return StateUpdateResult::CHANGED;
    }, "button_release");

    digitalWrite(_ledPin, LOW);
    wasPressed = false;

    // Pretty debug output
    Serial.println();
    Serial.println(F("BUTTON RELEASED"));
    Serial.print(F("   Duration   : "));
    Serial.print(duration);
    Serial.println(F(" ms"));
    Serial.print(F("   New state  : "));
    Serial.print(_state.durationMs);  // _state is accessible because we inherit StatefulService
    Serial.println(F(" ms (visible in UI)"));
    Serial.println();
  }

  // Optional: tiny debounce / responsiveness tweak (you can remove if not needed)
  // delay(5);
}