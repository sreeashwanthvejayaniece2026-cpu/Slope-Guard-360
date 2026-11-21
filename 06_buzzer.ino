// Slope Guard 360 — bench step 6 (node 2 only)
// Wiring: buzzer + to GPIO 33, buzzer - to GND.
//         A 3-pin module can use its I/O pin on GPIO 33 and its own VCC on 3V3.
// Pass: five clear beeps, then the sketch stops.
//
// BUZZER_IS_ACTIVE 1: module beeps from plain HIGH/LOW (most cheap modules).
// BUZZER_IS_ACTIVE 0: bare passive disc, driven with a 2 kHz tone.

#include <Arduino.h>

const int kBuzzer = 33;
const int BUZZER_IS_ACTIVE = 1;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Slope Guard bench 6 — buzzer");
  if (BUZZER_IS_ACTIVE) {
    pinMode(kBuzzer, OUTPUT);
    digitalWrite(kBuzzer, LOW);
  } else {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(kBuzzer, 2000, 8);
#else
    ledcSetup(4, 2000, 8);
    ledcAttachPin(kBuzzer, 4);
#endif
  }

  for (int i = 1; i <= 5; i++) {
    Serial.print("Beep ");
    Serial.println(i);
    if (BUZZER_IS_ACTIVE) {
      digitalWrite(kBuzzer, HIGH);
    } else {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
      ledcWrite(kBuzzer, 128);
#else
      ledcWrite(4, 128);
#endif
    }
    delay(200);
    if (BUZZER_IS_ACTIVE) {
      digitalWrite(kBuzzer, LOW);
    } else {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
      ledcWrite(kBuzzer, 0);
#else
      ledcWrite(4, 0);
#endif
    }
    delay(300);
  }
  Serial.println("Pass: you heard five beeps.");
}

void loop() {}
