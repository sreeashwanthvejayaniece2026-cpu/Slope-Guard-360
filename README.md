# Slope Guard 360 — source v1.0

Two ESP32 nodes watch a model pit slope. A React dashboard shows both benches. This folder is the complete source. Zip it and copy it to the laptop that will build and flash the boards. Nothing in this zip needs a password.

There is no `requirements.txt`. This is not a Python app. The boards are flashed with the Arduino IDE. The website is started with Node.js from the `dashboard` folder. Firebase is set up once in the browser. Those are three separate steps, and `pip install` will not do any of them.

The project monitors tilt, relative soil moisture, and surface wetness. It does not predict slope failure.

## Folder map

| Path | What it is |
|---|---|
| `SPEC.md` | Rules the firmware and dashboard follow |
| `firmware/bench/` | Seven small sketches. Flash them in order before the real program |
| `firmware/slope_guard/` | The one program both nodes run |
| `firmware/secrets.example.h` | Copy of the secrets file, with blanks |
| `firebase/database.rules.json` | Database rules to paste into Firebase |
| `firebase/SETUP.md` | Click-by-click Firebase setup |
| `dashboard/` | The website (Vite + React) |

## On the laptop, install these free tools

1. [Arduino IDE 2](https://www.arduino.cc/en/software).
2. ESP32 boards: Arduino IDE → File → Preferences → Additional boards manager URLs, add `https://espressif.github.io/arduino-esp32/package_esp32_index.json`. Then Boards Manager → install **esp32** by Espressif.
3. Library Manager, install: **Adafruit MPU6050**, **Adafruit Unified Sensor**, **Adafruit BusIO**, **ArduinoJson** (version 7 or newer).
4. [Node.js LTS](https://nodejs.org) (needed for the dashboard only).
5. A phone hotspot on **2.4 GHz**. On an iPhone turn on Maximize Compatibility. Use a password with letters and numbers only.

Board settings for every sketch: **ESP32 Dev Module**, upload speed **115200** if a faster speed fails, Serial Monitor **115200**. If the Serial Monitor stays blank on a native-USB board, set **USB CDC On Boot** to **Enabled**.

## Secrets (do this on the laptop, after unzip)

1. Copy `firmware/secrets.example.h` to `firmware/secrets.h`.
2. Put the hotspot name, hotspot password, and Firebase host in `secrets.h`.
3. Copy `dashboard/.env.example` to `dashboard/.env`.
4. Paste the Firebase web config into `.env`.

`secrets.h` and `.env` are listed in `.gitignore`. Do not put them back into a zip you upload to GitHub.

Firebase itself is created once in the browser. Follow `firebase/SETUP.md`.

## Build order

1. Breadboard node 1. Flash bench sketches `01` through `04`, then `07`. Stop when each pass condition in the sketch comment is true.
2. Breadboard node 2. Repeat, and also flash `05_rain` and `06_buzzer`.
3. Flash `firmware/slope_guard` to node 1 with the default settings (`node01`, no rain, no buzzer).
4. Change the four settings at the top of `firmware/slope_guard/config.h` for node 2, then flash node 2. Change them back before you flash node 1 again.
5. In a terminal opened inside `dashboard/`:

```
npm install
npm run dev
```

6. Open the local site, then deploy that same `dashboard` folder on Vercel (Hobby). Set the Vercel root directory to `dashboard`, and add the same `VITE_FIREBASE_*` values as environment variables.

## What you still measure yourselves

The moisture numbers in `config.h` are placeholders. After the soak test, replace `MOIST_RAW_DRY` and `MOIST_RAW_WET` on each node with that node's own readings. They will not match each other. Set `RAIN_RAW_WET_CUTOFF` between the dry-plate and wet-plate readings from bench sketch 05.
