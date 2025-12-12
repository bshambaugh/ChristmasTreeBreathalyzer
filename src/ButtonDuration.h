#ifndef BUTTON_DURATION_H
#define BUTTON_DURATION_H

#include <ArduinoJson.h>
#include <StatefulService.h>

class ButtonDuration {
public:
  unsigned long durationMs = 0;

  static void read(ButtonDuration& state, JsonObject& root) {
    root["duration_ms"] = state.durationMs;
  }

  static StateUpdateResult update(JsonObject& root, ButtonDuration& state) {
    if (root.containsKey("reset")) {
      state.durationMs = 0;
      return StateUpdateResult::CHANGED;
    }
    return StateUpdateResult::UNCHANGED;
  }
};

#endif
