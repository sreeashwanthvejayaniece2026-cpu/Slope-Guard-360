#pragma once

// ============================================================
//  Flash node 1 with the values below.
//  Before flashing node 2, change the three marked lines:
//    NODE_ID          "node02"
//    HAS_RAIN_SENSOR  1
//    HAS_BUZZER       1
//  Change them back before you flash node 1 again.
//  The Serial banner prints the node id after reset.
// ============================================================

#define NODE_ID "node01"       // node 2: "node02"
#define HAS_RAIN_SENSOR 0      // node 2: 1
#define HAS_BUZZER 0           // node 2: 1

// 1 = module that beeps when given a DC level (most 3-pin boards).
// 0 = bare passive disc that needs a tone.
#define BUZZER_IS_ACTIVE 1

// Placeholders. Replace with this node's own bench-4 readings.
// Dry soil maps to 0 percent. Soaked soil maps to 100 percent.
// The raw ADC value falls as the probe gets wetter.
#define MOIST_RAW_DRY 2800
#define MOIST_RAW_WET 1400

// Node 2 only. A raw reading at or below this counts as wet.
// Choose it between the dry-plate and wet-plate numbers from bench 5.
#define RAIN_RAW_WET_CUTOFF 2000

#define SEND_INTERVAL_MS 3000
#define CONFIG_POLL_MS 10000

#define TILT_CAUTION_DEFAULT 5.0f
#define TILT_WARNING_DEFAULT 10.0f
#define MOIST_CAUTION_DEFAULT 60.0f
#define MOIST_WARNING_DEFAULT 80.0f
#define OFFLINE_SECONDS_DEFAULT 15
