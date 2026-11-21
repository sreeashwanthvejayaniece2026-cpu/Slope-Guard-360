// Slope Guard 360 — bench step 7
// Wiring: ESP32 only. The hotspot must be 2.4 GHz.
// Pass: Serial prints "Wi-Fi joined" and an IP address. Turn the hotspot off
//       and on again. The sketch prints "Wi-Fi dropped" and then joins again.
//
// Uses firmware/secrets.h. Copy firmware/secrets.example.h there first.
// The password is never printed.

#include <Arduino.h>
#include <WiFi.h>

#if __has_include("../../secrets.h")
#include "../../secrets.h"
#else
#include "../../secrets.example.h"
#warning "Using secrets.example.h. Copy it to firmware/secrets.h and edit it."
#endif

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Slope Guard bench 7 — Wi-Fi");
  Serial.print("Joining hotspot: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.persistent(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  static bool wasUp = false;
  static uint32_t lastTry = 0;

  if (WiFi.status() == WL_CONNECTED) {
    if (!wasUp) {
      wasUp = true;
      Serial.print("Wi-Fi joined. IP ");
      Serial.println(WiFi.localIP());
    }
    delay(500);
    return;
  }

  if (wasUp) {
    wasUp = false;
    Serial.println("Wi-Fi dropped. Reconnecting.");
  } else if (millis() - lastTry > 5000) {
    lastTry = millis();
    Serial.println("Still trying to join. Reconnecting.");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  delay(200);
}
