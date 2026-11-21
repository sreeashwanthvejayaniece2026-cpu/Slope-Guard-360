#pragma once

// Experimental prototype thresholds. Not mine-safety limits.
//
// Levels: 0 normal, 1 caution, 2 warning.
// A level drops only after the reading falls 1.5 degrees (or 3 percent
// moisture) below the limit that set it. That is the hysteresis that
// stops the LED flickering when a value sits on the line.
//
// Example: tilt warning is 10 degrees and hysteresis is 1.5.
// The LED turns red at 10 degrees and stays red until the tilt is under 8.5.

struct Thresholds {
  float tiltCaution;
  float tiltWarning;
  float moistureCaution;
  float moistureWarning;
  int offlineSeconds;
};

struct Decision {
  int tiltLevel;
  int moistureLevel;
  int rainLevel;
  int status;
  const char* reason;
};

inline const char* statusName(int level) {
  if (level >= 2) return "warning";
  if (level == 1) return "caution";
  return "normal";
}

inline int nextLevel(float value, float caution, float warning, float hyst, int previous) {
  if (warning < caution) warning = caution;
  float cautionClear = caution - hyst;
  float warningClear = warning - hyst;
  if (cautionClear < 0) cautionClear = 0;
  if (warningClear < 0) warningClear = 0;

  if (previous >= 2) {
    if (value >= warningClear) return 2;
    if (value >= cautionClear) return 1;
    return 0;
  }
  if (previous == 1) {
    if (value >= warning) return 2;
    if (value >= cautionClear) return 1;
    return 0;
  }
  if (value >= warning) return 2;
  if (value >= caution) return 1;
  return 0;
}

// Called once per second. The static levels remember hysteresis.
// rainFitted is false on node 1, so a wet flag there is ignored.
inline Decision decide(float tiltDeg, float moisturePct, bool rainFitted, bool rainWet,
                       const Thresholds& limits) {
  static int tiltLevel = 0;
  static int moistureLevel = 0;

  tiltLevel = nextLevel(tiltDeg, limits.tiltCaution, limits.tiltWarning, 1.5f, tiltLevel);
  moistureLevel = nextLevel(moisturePct, limits.moistureCaution, limits.moistureWarning, 3.0f, moistureLevel);
  int rainLevel = (rainFitted && rainWet) ? 1 : 0;

  int status = tiltLevel;
  if (moistureLevel > status) status = moistureLevel;
  if (rainLevel > status) status = rainLevel;

  // Tilt-at-warning wins the reason so the bench-A tilt demo stays obvious.
  // moisture+rain is used when moisture is at warning and the plate is wet.
  const char* reason = "none";
  if (status > 0) {
    if (tiltLevel >= 2) reason = "tilt";
    else if (moistureLevel >= 2 && rainLevel >= 1) reason = "moisture+rain";
    else if (moistureLevel >= 2) reason = "moisture";
    else if (tiltLevel == 1) reason = "tilt";
    else if (moistureLevel == 1) reason = "moisture";
    else if (rainLevel == 1) reason = "rain";
  }

  Decision out;
  out.tiltLevel = tiltLevel;
  out.moistureLevel = moistureLevel;
  out.rainLevel = rainLevel;
  out.status = status;
  out.reason = reason;
  return out;
}
