// Slope Guard 360 — bench step 4
// Wiring: capacitive moisture VCC to 3V3, GND to GND, AOUT to GPIO 34.
//         GPIO 34 is ADC1, so the reading stays valid when Wi-Fi is on later.
// Pass: the raw number is different in air, in dry soil, and in water.
//       Write the dry-soil and soaked-soil numbers down. Raw should fall as
//       the sensor gets wetter. Those two numbers go into config.h.

#include <Arduino.h>

const int kMoisture = 34;

int average16() {
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogRead(kMoisture);
    delay(2);
  }
  return (int)(sum / 16);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(kMoisture, ADC_11db);
  delay(300);
  Serial.println("Slope Guard bench 4 — moisture");
  Serial.println("Pass: raw ADC changes between air, dry soil, and water.");
}

void loop() {
  int raw = average16();
  Serial.print("rawMoisture=");
  Serial.println(raw);
  delay(1000);
}
