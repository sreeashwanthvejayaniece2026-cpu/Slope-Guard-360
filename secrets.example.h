#pragma once

// Copy this file to firmware/secrets.h and edit the copy.
// secrets.h is gitignored. Do not put the real password in the example file.

// 2.4 GHz hotspot. Letters and numbers only in the password.
#define WIFI_SSID "slope-guard"
#define WIFI_PASSWORD "changeme"

// Host only, or the full https URL. No path after the host.
// Example: slope-guard-360-default-rtdb.asia-southeast1.firebasedatabase.app
#define FIREBASE_HOST "your-project-default-rtdb.asia-southeast1.firebasedatabase.app"

// Leave empty. The demo rules do not use a database secret.
#define FIREBASE_AUTH ""
