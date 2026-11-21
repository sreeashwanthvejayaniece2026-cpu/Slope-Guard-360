// Slope Guard 360 — bench step 2
// Wiring:
//   GPIO 25 -- 220 ohm -- green LED long leg -- short leg to GND
//   GPIO 26 -- 220 ohm -- yellow LED long leg -- short leg to GND
//   GPIO 27 -- 220 ohm -- red LED long leg -- short leg to GND
// Pass: the three LEDs light one at a time, in that order, and the Serial
// Monitor names the LED that should be on.

#include <Arduino.h>

const int kGreen = 25;
const int kYellow = 26;
const int kRed = 27;

void setup() {
  Serial.begin(115200);
  pinMode(kGreen, OUTPUT);
  pinMode(kYellow, OUTPUT);
  pinMode(kRed, OUTPUT);
  delay(300);
  Serial.println("Slope Guard bench 2 — LEDs");
  Serial.println("Pass: green, then yellow, then red.");
}

void light(int pin, const char* name) {
  digitalWrite(kGreen, pin == kGreen ? HIGH : LOW);
  digitalWrite(kYellow, pin == kYellow ? HIGH : LOW);
  digitalWrite(kRed, pin == kRed ? HIGH : LOW);
  Serial.println(name);
  delay(700);
}

void loop() {
  light(kGreen, "Green ON");
  light(kYellow, "Yellow ON");
  light(kRed, "Red ON");
}
