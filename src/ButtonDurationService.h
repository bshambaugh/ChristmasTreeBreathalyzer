#ifndef BUTTON_DURATION_SERVICE_H
#define BUTTON_DURATION_SERVICE_H

#include <Arduino.h>
#include <HttpEndpoint.h>
#include <WebSocketTxRx.h>
#include <SecurityManager.h>          // if you use authentication
#include "ButtonDuration.h"

#define BUTTON_DURATION_ENDPOINT_PATH "/rest/buttonDuration"
#define BUTTON_DURATION_WS_PATH "/ws/buttonDuration"

class ButtonDurationService : public StatefulService<ButtonDuration> {
public:
  ButtonDurationService(AsyncWebServer* server, SecurityManager* securityManager);

  void begin();
  void loop();
  void setupPins(int buttonPin, int ledPin);

private:
  HttpEndpoint<ButtonDuration> _httpEndpoint;
  WebSocketTxRx<ButtonDuration> _ws;

  int _buttonPin = -1;
  int _ledPin = -1;

  bool wasPressed = false;
  unsigned long pressStartTime = 0;
};

#endif
