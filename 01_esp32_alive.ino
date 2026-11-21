// Slope Guard 360 — bench step 1
// Wiring: ESP32 only, USB cable to the laptop. No sensors.
// Pass: Serial Monitor at 115200 prints a new "ESP32 is alive" line every second.

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("Slope Guard bench 1");
  Serial.println("Pass: a new line prints every second.");
}

void loop() {
  Serial.print("ESP32 is alive. millis=");
  Serial.println(millis());
  delay(1000);
}
