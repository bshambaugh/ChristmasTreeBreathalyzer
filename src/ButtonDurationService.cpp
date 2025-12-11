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
}

void ButtonDurationService::setupPins(int buttonPin, int ledPin) {
  _buttonPin = buttonPin;
  _ledPin = ledPin;

  pinMode(_buttonPin, INPUT_PULLUP);
  pinMode(_ledPin, OUTPUT);
}

void ButtonDurationService::begin() {
  _state.durationMs = 0;
}

void ButtonDurationService::loop() {
  if (_buttonPin < 0) return;

  bool pressed = !digitalRead(_buttonPin);  // active LOW

  if (pressed && !wasPressed) {
    pressStartTime = millis();
    digitalWrite(_ledPin, HIGH);
    wasPressed = true;
  }
  else if (!pressed && wasPressed) {

    // Correct update() usage
    this->update([&](ButtonDuration& state) {
      state.durationMs = millis() - pressStartTime;
      return StateUpdateResult::CHANGED;
    }, "button_release");

    digitalWrite(_ledPin, LOW);
    wasPressed = false;
  }
}
