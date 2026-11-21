// Slope Guard 360 — bench step 5 (node 2 only)
// Wiring: rain board VCC to GPIO 32 (not to 3V3), GND to GND, AO to GPIO 35.
//         Leave the DO pin unconnected.
//         GPIO 32 is switched on only while a reading is taken, so the plate
//         is not powered the rest of the time.
// Pass: rawRain is high when the plate is dry and clearly lower with a drop
//       of water on it. Pick a cutoff between those two numbers for config.h.

#include <Arduino.h>

const int kRainPower = 32;
const int kRainAnalog = 35;

int readRain() {
  digitalWrite(kRainPower, HIGH);
  delay(30);
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogRead(kRainAnalog);
    delay(2);
  }
  digitalWrite(kRainPower, LOW);
  return (int)(sum / 16);
}

void setup() {
  Serial.begin(115200);
  pinMode(kRainPower, OUTPUT);
  digitalWrite(kRainPower, LOW);
  analogReadResolution(12);
  analogSetPinAttenuation(kRainAnalog, ADC_11db);
  delay(300);
  Serial.println("Slope Guard bench 5 — rain plate");
  Serial.println("Pass: dry and wet readings are clearly different.");
}

void loop() {
  int raw = readRain();
  Serial.print("rawRain=");
  Serial.println(raw);
  delay(1000);
}
