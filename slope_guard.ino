// Slope Guard 360 — one sketch for both nodes.
//
// Libraries (Library Manager):
//   Adafruit MPU6050, Adafruit Unified Sensor, Adafruit BusIO, ArduinoJson 7.
// Board: ESP32 Dev Module. Serial Monitor: 115200.
//
// Set the node in config.h before you flash. Node 1 is the default.
// Wi-Fi and Firebase live in firmware/secrets.h (copy from secrets.example.h).
//
// setInsecure() is used below on purpose. This prototype does not check the
// Firebase certificate. Say that in the report. Do not reuse it on a real site.

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_task_wdt.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "pins.h"
#include "logic.h"

#if __has_include("../secrets.h")
#include "../secrets.h"
#else
#include "../secrets.example.h"
#warning "Using secrets.example.h. Copy it to firmware/secrets.h and edit it."
#endif

static const uint32_t kFastMs = 20;
static const uint32_t kStatusMs = 1000;
static const uint32_t kBaselineMs = 5000;
static const float kStillSpanDeg = 1.5f;
static const int kHistorySlots = 100;
static const int kAlertQueue = 8;

struct Sample {
  float tiltX;
  float tiltY;
  float tilt;
  float moisture;
  int rawMoisture;
  int rawRain;
  bool rainWet;
  char status[12];
  char reason[20];
  int rssi;
};

struct AlertItem {
  char parameter[20];
  char level[12];
};

struct PublishJob {
  uint8_t pending;
  uint32_t seq;
  Sample sample;
};

static Adafruit_MPU6050 gMpu;
static WiFiClientSecure gTls;
static SemaphoreHandle_t gMu;

static Thresholds gLimits = {
  TILT_CAUTION_DEFAULT,
  TILT_WARNING_DEFAULT,
  MOIST_CAUTION_DEFAULT,
  MOIST_WARNING_DEFAULT,
  OFFLINE_SECONDS_DEFAULT
};

static PublishJob gJob = {};
static AlertItem gAlerts[kAlertQueue];
static int gAlertHead = 0;
static int gAlertCount = 0;

static volatile uint8_t gWifiUp = 0;
static volatile int gRssi = 0;
static volatile int gHttpCode = 0;
static volatile uint8_t gRebaseline = 0;

static char gHost[100];
static bool gMpuOk = false;
static bool gBaselineReady = false;
static float gBaseRoll = 0;
static float gBasePitch = 0;

static float roundTo(float value, float scale) {
  return roundf(value * scale) / scale;
}

static void copyText(char* dest, size_t n, const char* src) {
  strncpy(dest, src, n - 1);
  dest[n - 1] = '\0';
}

static void lockData() {
  xSemaphoreTake(gMu, portMAX_DELAY);
}

static void unlockData() {
  xSemaphoreGive(gMu);
}

static Thresholds limitsCopy() {
  Thresholds copy;
  lockData();
  copy = gLimits;
  unlockData();
  return copy;
}

static void pushAlert(const char* parameter, const char* level) {
  lockData();
  int index = (gAlertHead + gAlertCount) % kAlertQueue;
  if (gAlertCount == kAlertQueue) {
    gAlertHead = (gAlertHead + 1) % kAlertQueue;
    gAlertCount--;
    index = (gAlertHead + gAlertCount) % kAlertQueue;
  }
  copyText(gAlerts[index].parameter, sizeof(gAlerts[index].parameter), parameter);
  copyText(gAlerts[index].level, sizeof(gAlerts[index].level), level);
  gAlertCount++;
  unlockData();
}

static bool popAlert(AlertItem* out) {
  bool have = false;
  lockData();
  if (gAlertCount > 0) {
    *out = gAlerts[gAlertHead];
    gAlertHead = (gAlertHead + 1) % kAlertQueue;
    gAlertCount--;
    have = true;
  }
  unlockData();
  return have;
}

static void queuePublish(const Sample& sample) {
  lockData();
  gJob.sample = sample;
  gJob.seq++;
  gJob.pending = 1;
  unlockData();
}

static float moisturePercent(float raw) {
  float dry = (float)MOIST_RAW_DRY;
  float wet = (float)MOIST_RAW_WET;
  if (fabsf(dry - wet) < 1.0f) return 0;
  float pct = (dry - raw) * 100.0f / (dry - wet);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}

static void prepareHost() {
  const char* in = FIREBASE_HOST;
  if (strncmp(in, "https://", 8) == 0) in += 8;
  else if (strncmp(in, "http://", 7) == 0) in += 7;
  strncpy(gHost, in, sizeof(gHost) - 1);
  gHost[sizeof(gHost) - 1] = '\0';
  size_t n = strlen(gHost);
  while (n > 0 && (gHost[n - 1] == '/' || gHost[n - 1] == ' ')) {
    gHost[--n] = '\0';
  }
}

static void buildUrl(char* out, size_t n, const char* path) {
  if (strlen(FIREBASE_AUTH) > 0) {
    const char* join = strchr(path, '?') ? "&" : "?";
    snprintf(out, n, "https://%s%s%sauth=%s", gHost, path, join, FIREBASE_AUTH);
  } else {
    snprintf(out, n, "https://%s%s", gHost, path);
  }
}

static void buzzerInit() {
#if HAS_BUZZER
  if (BUZZER_IS_ACTIVE) {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
  } else {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(PIN_BUZZER, 2000, 8);
#else
    ledcSetup(4, 2000, 8);
    ledcAttachPin(PIN_BUZZER, 4);
#endif
  }
#endif
}

static void buzzerSet(bool on) {
#if HAS_BUZZER
  if (BUZZER_IS_ACTIVE) {
    digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
  } else {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(PIN_BUZZER, on ? 128 : 0);
#else
    ledcWrite(4, on ? 128 : 0);
#endif
  }
#else
  (void)on;
#endif
}

static void ledsOff() {
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);
}

static void applyOutputs(uint32_t now, int level, bool startup) {
  if (startup) {
    bool on = (now < 200);
    digitalWrite(PIN_LED_GREEN, on ? HIGH : LOW);
    digitalWrite(PIN_LED_YELLOW, on ? HIGH : LOW);
    digitalWrite(PIN_LED_RED, on ? HIGH : LOW);
    buzzerSet(false);
    return;
  }

  if (!gMpuOk) {
    bool blink = ((now / 120) % 2) == 0;
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, blink ? HIGH : LOW);
    buzzerSet(false);
    return;
  }

  // Wi-Fi down: the status LED blinks instead of staying on.
  bool show = gWifiUp || (((now / 500) % 2) == 0);
  digitalWrite(PIN_LED_GREEN, (level == 0 && show) ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, (level == 1 && show) ? HIGH : LOW);
  digitalWrite(PIN_LED_RED, (level == 2 && show) ? HIGH : LOW);

#if HAS_BUZZER
  bool beep = (level >= 2) && ((now % 600) < 200);
  buzzerSet(beep);
#else
  buzzerSet(false);
#endif
}

static bool readAngles(float* roll, float* pitch) {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  if (!gMpu.getEvent(&accel, &gyro, &temp)) return false;
  *roll = atan2f(accel.acceleration.y, accel.acceleration.z) * 57.2957795f;
  *pitch = atan2f(-accel.acceleration.x,
                  sqrtf(accel.acceleration.y * accel.acceleration.y +
                        accel.acceleration.z * accel.acceleration.z)) *
           57.2957795f;
  return true;
}

static void startBaseline() {
  gBaselineReady = false;
}

#if ARDUINOJSON_VERSION_MAJOR >= 7
static void fillReading(JsonObject obj, const Sample& sample) {
#else
static void fillReading(JsonObject& obj, const Sample& sample) {
#endif
  obj["nodeId"] = NODE_ID;
  obj["tiltX"] = roundTo(sample.tiltX, 100.0f);
  obj["tiltY"] = roundTo(sample.tiltY, 100.0f);
  obj["tilt"] = roundTo(sample.tilt, 100.0f);
  obj["moisture"] = roundTo(sample.moisture, 10.0f);
  obj["rawMoisture"] = sample.rawMoisture;
#if HAS_RAIN_SENSOR
  obj["rainWet"] = sample.rainWet;
  obj["rawRain"] = sample.rawRain;
#endif
  obj["status"] = sample.status;
  obj["reason"] = sample.reason;
  obj["rssi"] = sample.rssi;
  obj["ts"][".sv"] = "timestamp";
}

static int httpSend(const char* method, const char* path, const char* body, String* response) {
  if (!gWifiUp) return -1;
  if (strstr(gHost, "your-project") != nullptr) {
    static bool warned = false;
    if (!warned) {
      Serial.println("Firebase host is still the example. Edit firmware/secrets.h.");
      warned = true;
    }
    return -2;
  }

  char url[220];
  buildUrl(url, sizeof(url), path);

  HTTPClient http;
  http.setReuse(true);
  http.setTimeout(8000);
  http.setConnectTimeout(8000);
  if (!http.begin(gTls, url)) return -3;
  http.addHeader("Content-Type", "application/json");

  int code = http.sendRequest(method, body);
  if (response != nullptr && code > 0) *response = http.getString();
  http.end();
  if (code <= 0) gTls.stop();
  return code;
}

static bool publishSample(const Sample& sample, int slot) {
#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  DynamicJsonDocument doc(1024);
#endif
  char nodePath[40];
  char histPath[48];
  snprintf(nodePath, sizeof(nodePath), "nodes/%s", NODE_ID);
  snprintf(histPath, sizeof(histPath), "history/%s/%d", NODE_ID, slot);

#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonObject node = doc[nodePath].to<JsonObject>();
  JsonObject hist = doc[histPath].to<JsonObject>();
  fillReading(node, sample);
  fillReading(hist, sample);
#else
  JsonObject node = doc.createNestedObject(nodePath);
  JsonObject hist = doc.createNestedObject(histPath);
  fillReading(node, sample);
  fillReading(hist, sample);
#endif

  char body[900];
  size_t n = serializeJson(doc, body, sizeof(body));
  if (n == 0 || n >= sizeof(body)) return false;

  int code = httpSend("PATCH", "/.json", body, nullptr);
  gHttpCode = code;
  return code >= 200 && code < 300;
}

static bool postAlert(const AlertItem& alert) {
#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  DynamicJsonDocument doc(256);
#endif
  doc["nodeId"] = NODE_ID;
  doc["parameter"] = alert.parameter;
  doc["level"] = alert.level;
  doc["ts"][".sv"] = "timestamp";

  char body[220];
  size_t n = serializeJson(doc, body, sizeof(body));
  if (n == 0 || n >= sizeof(body)) return false;
  int code = httpSend("POST", "/alerts.json", body, nullptr);
  return code >= 200 && code < 300;
}

static void applyThresholds(JsonObject obj) {
  if (!obj["tiltCaution"].is<float>() && !obj["tiltCaution"].is<int>()) return;
  float tiltCaution = obj["tiltCaution"] | 0.0f;
  float tiltWarning = obj["tiltWarning"] | 0.0f;
  float moistureCaution = obj["moistureCaution"] | 0.0f;
  float moistureWarning = obj["moistureWarning"] | 0.0f;
  int offlineSeconds = obj["offlineSeconds"] | 0;

  bool tiltOk = tiltCaution > 0 && tiltCaution < 45 && tiltWarning > tiltCaution && tiltWarning <= 90;
  bool moistOk = moistureCaution >= 0 && moistureCaution < 100 &&
                 moistureWarning > moistureCaution && moistureWarning <= 100;
  bool offlineOk = offlineSeconds >= 5 && offlineSeconds <= 300;
  if (!tiltOk || !moistOk || !offlineOk) {
    Serial.println("Ignored a thresholds update outside the allowed range.");
    return;
  }

  lockData();
  gLimits.tiltCaution = tiltCaution;
  gLimits.tiltWarning = tiltWarning;
  gLimits.moistureCaution = moistureCaution;
  gLimits.moistureWarning = moistureWarning;
  gLimits.offlineSeconds = offlineSeconds;
  unlockData();
}

static void pollConfig() {
  String payload;
  int code = httpSend("GET", "/thresholds.json", "", &payload);
  if (code == 200 && payload != "null" && payload.length() > 2) {
#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    if (!deserializeJson(doc, payload)) applyThresholds(doc.as<JsonObject>());
  }

  char path[40];
  snprintf(path, sizeof(path), "/commands/%s.json", NODE_ID);
  payload = "";
  code = httpSend("GET", path, "", &payload);
  if (code != 200 || payload == "null") return;

#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  DynamicJsonDocument doc(256);
#endif
  if (deserializeJson(doc, payload)) return;

  long counter = doc["rebaseline"] | 0L;
  static bool seen = false;
  static long last = 0;
  if (!seen) {
    last = counter;
    seen = true;
    return;
  }
  if (counter != last) {
    last = counter;
    gRebaseline = 1;
    Serial.println("Re-baseline requested by the dashboard.");
  }
}

static void maintainWifi(uint32_t now) {
  static uint32_t lastTry = 0;
  static uint32_t backoff = 1000;
  static uint32_t boot = millis();

  bool up = WiFi.status() == WL_CONNECTED;
  gWifiUp = up ? 1 : 0;
  gRssi = up ? WiFi.RSSI() : 0;
  if (up) {
    backoff = 1000;
    return;
  }

  // Let the first begin() finish. Calling disconnect immediately would cancel it.
  if (now - boot < 12000) return;
  if (now - lastTry < backoff) return;
  lastTry = now;
  Serial.print("Wi-Fi down. Reconnecting. Next wait ");
  Serial.print(backoff);
  Serial.println(" ms");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (backoff < 30000) backoff *= 2;
  gTls.stop();
}

static void networkTask(void* unused) {
  (void)unused;
  uint32_t lastPoll = 0;
  int slot = 0;

  for (;;) {
    uint32_t now = millis();
    maintainWifi(now);

    AlertItem alert;
    if (gWifiUp && popAlert(&alert)) {
      if (!postAlert(alert)) {
        // Put it back at the head by pushing only if the queue has room.
        // A failed post is retried by re-queueing at the tail.
        pushAlert(alert.parameter, alert.level);
        vTaskDelay(pdMS_TO_TICKS(500));
      }
    }

    Sample sample;
    uint32_t seq = 0;
    bool pending = false;
    lockData();
    if (gJob.pending) {
      sample = gJob.sample;
      seq = gJob.seq;
      gJob.pending = 0;
      pending = true;
    }
    unlockData();

    if (pending) {
      sample.rssi = gRssi;
      bool ok = publishSample(sample, slot);
      if (ok) {
        slot = (slot + 1) % kHistorySlots;
      } else {
        lockData();
        if (gJob.seq == seq) gJob.pending = 1;
        unlockData();
      }
    }

    if (gWifiUp && now - lastPoll >= CONFIG_POLL_MS) {
      lastPoll = now;
      pollConfig();
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("Slope Guard 360");
  Serial.print("Node: ");
  Serial.println(NODE_ID);
  Serial.print("Rain sensor: ");
  Serial.println(HAS_RAIN_SENSOR ? "yes" : "no");
  Serial.print("Buzzer: ");
  Serial.println(HAS_BUZZER ? "yes" : "no");

  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  ledsOff();
  pinMode(PIN_RAIN_POWER, OUTPUT);
  digitalWrite(PIN_RAIN_POWER, LOW);
  buzzerInit();

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_MOIST, ADC_11db);
#if HAS_RAIN_SENSOR
  analogSetPinAttenuation(PIN_RAIN, ADC_11db);
#endif

  Wire.begin(PIN_SDA, PIN_SCL);
  gMpuOk = gMpu.begin(0x68, &Wire);
  if (gMpuOk) {
    gMpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    gMpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 ready.");
  } else {
    Serial.println("MPU6050 not found. Check SDA GPIO21, SCL GPIO22, 3V3, GND.");
  }

  prepareHost();
  // Prototype only: skip the TLS certificate check. See the file header.
  gTls.setInsecure();
  gTls.setTimeout(8);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Joining hotspot: ");
  Serial.println(WIFI_SSID);

  gMu = xSemaphoreCreateMutex();
  enableLoopWDT();

  xTaskCreatePinnedToCore(networkTask, "sgnet", 20480, nullptr, 1, nullptr, 0);
}

void loop() {
  esp_task_wdt_reset();
  uint32_t now = millis();

  static uint32_t lastFast = 0;
  static uint32_t lastStatus = 0;
  static uint32_t lastMpuTry = 0;

  static float rollSum = 0, pitchSum = 0, moistSum = 0;
  static int fastCount = 0;
  static int rawMoistLast = 0;

  static float secRoll = 0, secPitch = 0, secMoist = 0;
  static int secRaw = 0;
  static int secRainRaw = 0;
  static bool secRainWet = false;
  static int secondsInWindow = 0;
  static float winRoll = 0, winPitch = 0, winMoist = 0;
  static int winRaw = 0, winRainRaw = 0, winRainWet = 0, winRainSamples = 0;

  static uint32_t baseStart = 0;
  static float baseRollSum = 0, basePitchSum = 0;
  static float baseRollMin = 0, baseRollMax = 0, basePitchMin = 0, basePitchMax = 0;
  static int baseCount = 0;
  static bool baseOpen = false;

  static int rainState = 0;
  static uint32_t rainOnAt = 0;
  static uint32_t lastRain = 0;
  static int rainRaw = 0;
  static bool rainWet = false;
  static int rainSticky = 0;

  static char lastStatusName[12] = "";
  static char lastReasonName[20] = "";
  static bool alertPrimed = false;
  static int shownLevel = 0;

  bool startup = now < 400;

  if (!gMpuOk && now - lastMpuTry > 2000) {
    lastMpuTry = now;
    gMpuOk = gMpu.begin(0x68, &Wire);
    if (gMpuOk) {
      gMpu.setAccelerometerRange(MPU6050_RANGE_2_G);
      gMpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
      Serial.println("MPU6050 found.");
      baseOpen = false;
      gBaselineReady = false;
    }
  }

  if (gRebaseline) {
    gRebaseline = 0;
    baseOpen = false;
    gBaselineReady = false;
    Serial.println("Capturing a new baseline. Hold the model still.");
  }

  if (gMpuOk && now - lastFast >= kFastMs) {
    lastFast = now;
    float roll = 0, pitch = 0;
    if (readAngles(&roll, &pitch)) {
      int raw = analogRead(PIN_MOIST);
      rawMoistLast = raw;
      rollSum += roll;
      pitchSum += pitch;
      moistSum += raw;
      fastCount++;

      if (!baseOpen) {
        baseOpen = true;
        baseStart = now;
        baseRollSum = 0;
        basePitchSum = 0;
        baseRollMin = roll;
        baseRollMax = roll;
        basePitchMin = pitch;
        basePitchMax = pitch;
        baseCount = 0;
      }
      baseRollSum += roll;
      basePitchSum += pitch;
      if (roll < baseRollMin) baseRollMin = roll;
      if (roll > baseRollMax) baseRollMax = roll;
      if (pitch < basePitchMin) basePitchMin = pitch;
      if (pitch > basePitchMax) basePitchMax = pitch;
      baseCount++;

      bool quiet = (baseRollMax - baseRollMin) < kStillSpanDeg &&
                   (basePitchMax - basePitchMin) < kStillSpanDeg;
      if (now - baseStart >= kBaselineMs && baseCount > 20 && quiet) {
        gBaseRoll = baseRollSum / baseCount;
        gBasePitch = basePitchSum / baseCount;
        gBaselineReady = true;
        baseOpen = false;
        Serial.print("Baseline set. roll=");
        Serial.print(gBaseRoll, 2);
        Serial.print(" pitch=");
        Serial.println(gBasePitch, 2);
      } else if (now - baseStart >= kBaselineMs && !quiet) {
        baseOpen = false;
        Serial.println("Baseline restarted. The model moved.");
      }
    }
  }

#if HAS_RAIN_SENSOR
  if (rainState == 0 && now - lastRain >= 1000) {
    digitalWrite(PIN_RAIN_POWER, HIGH);
    rainOnAt = now;
    rainState = 1;
  } else if (rainState == 1 && now - rainOnAt >= 30) {
    rainRaw = analogRead(PIN_RAIN);
    digitalWrite(PIN_RAIN_POWER, LOW);
    // Wet reads lower. 150 counts of hysteresis stop a drop from flickering.
    if (rainSticky == 0) {
      if (rainRaw <= RAIN_RAW_WET_CUTOFF) rainSticky = 1;
    } else if (rainRaw >= RAIN_RAW_WET_CUTOFF + 150) {
      rainSticky = 0;
    }
    rainWet = rainSticky == 1;
    rainState = 0;
    lastRain = now;
  }
#endif

  if (now - lastStatus >= kStatusMs) {
    lastStatus = now;
    if (fastCount > 0) {
      secRoll = rollSum / fastCount;
      secPitch = pitchSum / fastCount;
      secRaw = (int)(moistSum / fastCount);
      secMoist = moisturePercent((float)secRaw);
      rollSum = pitchSum = moistSum = 0;
      fastCount = 0;
    }
    secRainRaw = rainRaw;
    secRainWet = rainWet;

    float dX = 0, dY = 0, tilt = 0;
    if (gBaselineReady) {
      dX = secRoll - gBaseRoll;
      dY = secPitch - gBasePitch;
      tilt = fabsf(dX);
      if (fabsf(dY) > tilt) tilt = fabsf(dY);
    }

    Decision decision = decide(tilt, secMoist, HAS_RAIN_SENSOR, secRainWet, limitsCopy());
    shownLevel = decision.status;

    if (!alertPrimed) {
      alertPrimed = true;
      copyText(lastStatusName, sizeof(lastStatusName), statusName(decision.status));
      copyText(lastReasonName, sizeof(lastReasonName), decision.reason);
      if (strcmp(lastStatusName, "normal") != 0) {
        pushAlert(decision.reason, lastStatusName);
      }
    } else if (strcmp(lastStatusName, statusName(decision.status)) != 0 ||
               strcmp(lastReasonName, decision.reason) != 0) {
      copyText(lastStatusName, sizeof(lastStatusName), statusName(decision.status));
      copyText(lastReasonName, sizeof(lastReasonName), decision.reason);
      pushAlert(decision.reason, lastStatusName);
    }

    winRoll += secRoll;
    winPitch += secPitch;
    winMoist += secMoist;
    winRaw += secRaw;
    secondsInWindow++;
#if HAS_RAIN_SENSOR
    winRainRaw += secRainRaw;
    winRainWet += secRainWet ? 1 : 0;
    winRainSamples++;
#endif

    if (secondsInWindow >= (SEND_INTERVAL_MS / 1000)) {
      Sample sample = {};
      float n = (float)secondsInWindow;
      float avgRoll = winRoll / n;
      float avgPitch = winPitch / n;
      if (gBaselineReady) {
        sample.tiltX = avgRoll - gBaseRoll;
        sample.tiltY = avgPitch - gBasePitch;
        sample.tilt = fabsf(sample.tiltX);
        if (fabsf(sample.tiltY) > sample.tilt) sample.tilt = fabsf(sample.tiltY);
      }
      sample.moisture = winMoist / n;
      sample.rawMoisture = winRaw / secondsInWindow;
#if HAS_RAIN_SENSOR
      sample.rawRain = winRainSamples ? (winRainRaw / winRainSamples) : 0;
      sample.rainWet = winRainWet > 0;
#endif
      sample.rssi = gRssi;
      copyText(sample.status, sizeof(sample.status), statusName(decision.status));
      copyText(sample.reason, sizeof(sample.reason), decision.reason);
      queuePublish(sample);

      winRoll = winPitch = winMoist = 0;
      winRaw = winRainRaw = winRainWet = winRainSamples = 0;
      secondsInWindow = 0;
    }

    Serial.print(NODE_ID);
    Serial.print(" roll=");
    Serial.print(secRoll, 2);
    Serial.print(" pitch=");
    Serial.print(secPitch, 2);
    Serial.print(" dX=");
    Serial.print(gBaselineReady ? (secRoll - gBaseRoll) : 0.0f, 2);
    Serial.print(" dY=");
    Serial.print(gBaselineReady ? (secPitch - gBasePitch) : 0.0f, 2);
    Serial.print(" tilt=");
    Serial.print(tilt, 2);
    Serial.print(" moist=");
    Serial.print(secMoist, 1);
    Serial.print(" raw=");
    Serial.print(secRaw);
    Serial.print(" rain=");
#if HAS_RAIN_SENSOR
    Serial.print(secRainWet ? "wet" : "dry");
    Serial.print(" rawRain=");
    Serial.print(secRainRaw);
#else
    Serial.print("na");
#endif
    Serial.print(" status=");
    Serial.print(statusName(decision.status));
    Serial.print(" reason=");
    Serial.print(decision.reason);
    Serial.print(" wifi=");
    Serial.print(gWifiUp ? 1 : 0);
    Serial.print(" rssi=");
    Serial.print(gRssi);
    Serial.print(" base=");
    Serial.print(gBaselineReady ? "set" : "waiting");
    Serial.print(" http=");
    Serial.println(gHttpCode);
  }

  applyOutputs(now, shownLevel, startup);

  // Yield so the idle task can run. The watchdog watches this loop and the idle task.
  // delay() is not used here, so the LEDs are not stuck behind a network call.
  vTaskDelay(pdMS_TO_TICKS(5));
}
