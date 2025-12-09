#include <Arduino.h>
/* with the pull-up resistor*/


int buttonPin = 8; //5
int Led = 10;

unsigned long time_value;

void setup() {
pinMode(buttonPin,INPUT);
pinMode(Led,OUTPUT);
Serial.begin(115200);
}

void loop() {
int buttonState = digitalRead(buttonPin); //read the state of the button input
if (buttonState == LOW) { //pressing the button will produce a LOW state 0V
digitalWrite(Led,HIGH); //the led with turn on
Serial.println(buttonState);
} else{
digitalWrite(Led,LOW); //the led with turn off
} Serial.println(buttonState); //check in the serial monitor
}

/*

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

*/