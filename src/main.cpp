#include <Arduino.h>

void setup() {
   pinMode(LED_BUILTIN, HIGH);
   Serial.begin(115200);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("I am high");
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("I am sober");
  delay(1000);
}

