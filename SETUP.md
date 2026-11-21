# Slope Guard 360 — setup steps

Follow this from top to bottom on the personal laptop. The source is the folder `slope_guard_codebase_v1_0` inside `D:\PERSONAL\pro_pr\new_proj`.

This is not a Python project. There is no `requirements.txt`. `pip install` does not build the boards or the website.

You will do four different jobs:

1. Put the folder on the laptop and on GitHub.
2. Create the free Firebase database in the browser.
3. Flash the ESP32 boards with the Arduino IDE.
4. Start the dashboard with Node.js, then put that same site on Vercel.

Do not put the Wi-Fi password or the Firebase keys into GitHub.

---

## 1. Copy the code to the laptop

1. On this PC, zip the folder `D:\PERSONAL\pro_pr\new_proj\slope_guard_codebase_v1_0`.
2. Zip it before anyone creates `firmware\secrets.h` or `dashboard\.env`. Those files hold the password and the Firebase keys.
3. Copy the zip to the personal laptop (USB drive or a chat file to yourself).
4. Unzip it. You should see one folder named `slope_guard_codebase_v1_0` containing `firmware`, `dashboard`, `firebase`, `README.md`, and `SPEC.md`.
5. Keep this setup note with the zip, or copy `SLOPE_GUARD_SETUP.md` into that folder so the team still has the steps after the zip move.

Check the unzipped folder contains these paths:

- `firmware\slope_guard\slope_guard.ino`
- `firmware\bench\01_esp32_alive\01_esp32_alive.ino`
- `firmware\secrets.example.h`
- `dashboard\package.json`
- `dashboard\.env.example`
- `firebase\database.rules.json`

---

## 2. Install the free tools

Install all of these on the laptop before opening the code.

### Arduino IDE

1. Download Arduino IDE 2 from https://www.arduino.cc/en/software and install it.
2. Open Arduino IDE.
3. File → Preferences.
4. In **Additional boards manager URLs** paste this line and click OK:

   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`

5. Open the Boards Manager (left icon that looks like a chip). Search `esp32`. Install **esp32 by Espressif Systems**.
6. Open the Library Manager. Install these four, and accept the dependencies the IDE offers:

   - Adafruit MPU6050
   - Adafruit Unified Sensor
   - Adafruit BusIO
   - ArduinoJson — version 7 or newer

`firmware\libraries.txt` is only that same list. It is not something you install with pip.

### Node.js

1. Download the LTS installer from https://nodejs.org and install it.
2. Close and reopen the terminal.
3. Check it worked:

```
node -v
npm -v
```

Both commands should print a version number.

### Phone hotspot

1. Turn on a phone hotspot.
2. It must be **2.4 GHz**. An ESP32 cannot join a 5 GHz-only network.
3. On an iPhone, enable **Maximize Compatibility**.
4. Use a password made of letters and numbers only. No spaces or symbols.
5. Give the hotspot a short fixed name, for example `sloeguard`. Write the name and password down. Both boards and the laptop will join this hotspot on demo day.

College Wi-Fi usually has a login page. The ESP32 cannot pass that page, so the hotspot is the demo network.

### Git and GitHub

1. Install Git from https://git-scm.com/download/win if `git --version` fails in a terminal.
2. Create a GitHub account if the team does not have one.
3. During the first `git push`, the browser will ask you to sign in to GitHub. That is expected.

---

## 3. Put the clean folder on GitHub

Do this before you fill in passwords. The first commit should contain only the example files.

1. On GitHub, click **New repository**.
2. Name it `slope-guard-360`.
3. Leave it empty. Do not add a README, a license, or a `.gitignore`. Those files are already in the folder.
4. On the laptop, open PowerShell in the unzipped `slope_guard_codebase_v1_0` folder. Shift-right-click the folder and choose **Open in Terminal**, or run:

```
cd C:\path\to\slope_guard_codebase_v1_0
```

5. Run these commands. Replace `YOUR-USERNAME` with the GitHub username:

```
git init
git status
git add .
git commit -m "Initial Slope Guard 360 source"
git branch -M main
git remote add origin https://github.com/YOUR-USERNAME/slope-guard-360.git
git push -u origin main
```

6. Read `git status` before `git add .`. The list must not contain `firmware\secrets.h` or `dashboard\.env`. A fresh unzip will not have those files. If they appear later, do not continue. They are already named in `.gitignore`, but only if you created them after the copy step.

7. Do not use the GitHub website button **Upload files**. It does not keep folders.

After this, every later code change is saved with:

```
git status
git add .
git commit -m "Short note of what changed"
git push
```

Teammates get those changes with `git pull` inside the same folder.

---

## 4. Create Firebase

Do this once in the browser. Add the other three teammates as owners.

1. Open https://console.firebase.google.com and create a project named `slope-guard-360`. Leave Analytics off.
2. Build → Realtime Database → Create Database.
3. Choose the Singapore region if it is offered (`asia-southeast1`).
4. Choose **locked mode**. Do not use test mode. Test mode expires after about 30 days and the boards will silently stop writing.
5. Open the **Rules** tab. Delete everything there. Paste the full contents of `firebase\database.rules.json`. Click **Publish**.
6. Copy the database URL. It looks like:

   `https://slope-guard-360-default-rtdb.asia-southeast1.firebasedatabase.app`

7. Project settings (gear) → General → Your apps → Add app → Web (`</>`).
8. Register the app as `slope-guard-dashboard`.
9. Copy the config values. You need `apiKey`, `authDomain`, `databaseURL`, `projectId`, and `appId`.
10. Project settings → Users and permissions. Invite the other three members as **Owner**.

Anyone who knows this URL can write readings. That is acceptable for a short college demo. Write that limitation in the report. It is not acceptable for a real mine.

A week before the viva, open the Rules tab again and confirm the published rules are still the file from the repo, not leftover test mode.

---

## 5. Fill the two secret files on the laptop

These files stay on the laptop. Git is set to ignore them. Do not paste them into chat or into GitHub.

### Firmware

1. In `firmware`, copy `secrets.example.h` to `secrets.h`:

```
copy firmware\secrets.example.h firmware\secrets.h
```

2. Open `firmware\secrets.h` and set:

```
#define WIFI_SSID "sloeguard"
#define WIFI_PASSWORD "yourhotspotpassword"
#define FIREBASE_HOST "slope-guard-360-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH ""
```

`FIREBASE_HOST` is the database host only. You may paste the full `https://...` URL. Do not add `.json` or a path after the host. Leave `FIREBASE_AUTH` as empty quotes.

### Dashboard

1. In `dashboard`, copy the example env file:

```
copy dashboard\.env.example dashboard\.env
```

2. Open `dashboard\.env` and paste the web config:

```
VITE_FIREBASE_API_KEY=paste-api-key
VITE_FIREBASE_AUTH_DOMAIN=slope-guard-360.firebaseapp.com
VITE_FIREBASE_DATABASE_URL=https://slope-guard-360-default-rtdb.asia-southeast1.firebasedatabase.app
VITE_FIREBASE_PROJECT_ID=slope-guard-360
VITE_FIREBASE_APP_ID=paste-app-id
```

The database URL here keeps the `https://`. No quotes. No spaces around the `=`.

---

## 6. Arduino settings for every sketch

Use these for the bench sketches and for the final program.

1. Tools → Board → esp32 → **ESP32 Dev Module**.
2. Tools → Port → the COM port that appears when the USB cable is plugged in. If no port appears, try another cable. Many cables charge only and do not carry data.
3. If upload fails, Tools → Upload Speed → **115200**.
4. Tools → Serial Monitor, then set the baud rate to **115200**.
5. If the monitor stays blank on a board whose USB socket goes straight into the ESP32 (no separate USB chip), set **USB CDC On Boot** to **Enabled** and upload again.

Open a sketch by opening its `.ino` file. The folder name and the file name must match. Do not rename them. Do not merge the bench sketches into one file.

---

## 7. Wire and test node 1 on a breadboard

Node 1 is bench A. It has the ESP32, one MPU6050, one moisture sensor, and three LEDs. No rain board and no buzzer.

Power every sensor from **3V3**, not from 5V or VIN. Join all grounds to one GND rail.

| From | To |
|---|---|
| MPU6050 VCC | 3V3 |
| MPU6050 GND | GND |
| MPU6050 SDA | GPIO 21 |
| MPU6050 SCL | GPIO 22 |
| Moisture VCC | 3V3 |
| Moisture GND | GND |
| Moisture AOUT | GPIO 34 |
| GPIO 25 | 220 ohm, then green LED long leg |
| GPIO 26 | 220 ohm, then yellow LED long leg |
| GPIO 27 | 220 ohm, then red LED long leg |
| All three LED short legs | GND |

Leave the MPU6050 AD0 and INT pins unconnected. Use the GPIO numbers printed on the board, not the pin’s position in the row.

Flash these sketches one at a time. The pass line is in the comment at the top of each file. Do not go on until that line is true.

1. `firmware\bench\01_esp32_alive\01_esp32_alive.ino` — a new “ESP32 is alive” line every second.
2. `firmware\bench\02_leds\02_leds.ino` — green, then yellow, then red.
3. `firmware\bench\03_mpu6050\03_mpu6050.ino` — it finds address `0x68`, and roll/pitch change when you tilt the board.
4. `firmware\bench\04_moisture\04_moisture.ino` — the raw number is different in air, dry soil, and water. Write down the dry-soil number and the soaked-soil number for this sensor.
5. `firmware\bench\07_wifi\07_wifi.ino` — after `secrets.h` is filled, it prints “Wi-Fi joined” and an IP address. Turn the hotspot off and on. It must join again.

---

## 8. Wire and test node 2

Node 2 is bench B. Build it as a second copy of node 1, then add the rain board and the buzzer.

| From | To |
|---|---|
| Rain board VCC | GPIO 32 (not 3V3) |
| Rain board GND | GND |
| Rain board AO | GPIO 35 |
| Buzzer + | GPIO 33 |
| Buzzer − | GND |

Leave the rain board DO pin unconnected. GPIO 32 powers the plate only while a reading is taken.

Flash the same node-1 sketches again on this second board (`01`, `02`, `03`, `04`, `07`) so you know this ESP32 is good. Then flash:

6. `firmware\bench\05_rain\05_rain.ino` — dry plate and a plate with a drop of water give clearly different raw numbers. Write both down.
7. `firmware\bench\06_buzzer\06_buzzer.ino` — five beeps. If the board resets when the buzzer sounds, the buzzer is drawing too much current. See the transistor note at the bottom of `firmware\README.md`.

The two moisture sensors will not read the same number. Keep a separate dry and soaked pair for node 1 and for node 2.

---

## 9. Flash the real program

The file is `firmware\slope_guard\slope_guard.ino`. One program runs on both boards. `firmware\slope_guard\config.h` says which board it is.

### Node 1

Leave `config.h` as it is shipped:

```
#define NODE_ID "node01"
#define HAS_RAIN_SENSOR 0
#define HAS_BUZZER 0
```

Put this board’s dry and soaked moisture numbers into:

```
#define MOIST_RAW_DRY 2800
#define MOIST_RAW_WET 1400
```

Replace `2800` and `1400` with the numbers you wrote down from bench step 4 on **this** sensor. Dry maps to 0 percent. Soaked maps to 100 percent. The raw number should be lower when the sensor is wet.

Upload to the node 1 board. Open the Serial Monitor at 115200. The first lines must say `Node: node01`. Hold the board still. Within a few seconds the line should show `base=set`. About every 3 seconds `http=200` means Firebase accepted a reading.

### Node 2

Edit the same `config.h` before this upload:

```
#define NODE_ID "node02"
#define HAS_RAIN_SENSOR 1
#define HAS_BUZZER 1
```

Set `MOIST_RAW_DRY` and `MOIST_RAW_WET` to **node 2’s** bench-4 numbers, not node 1’s. Set `RAIN_RAW_WET_CUTOFF` to a number between the dry-plate reading and the wet-plate reading from bench step 5. A reading at or below that cutoff counts as wet.

`BUZZER_IS_ACTIVE` stays `1` if the module beeps when you touch it to 3V3 and GND. Set it to `0` only for a bare passive disc that just clicks.

Upload to the node 2 board. The banner must say `Node: node02`.

Change `config.h` back to the node 1 values before you ever upload to node 1 again. The banner is the check. If the wrong id is printed, that board has the other node’s program.

In the Firebase console, open Realtime Database → Data. You should see `nodes/node01` and `nodes/node02` updating.

---

## 10. What the serial line is telling you

One line per second looks like this:

`node01 roll=… pitch=… dX=… dY=… tilt=… moist=… raw=… rain=… status=… reason=… wifi=… rssi=… base=… http=…`

| Piece | Meaning |
|---|---|
| `roll`, `pitch` | Angle of the sensor right now |
| `dX`, `dY` | Change from the baseline |
| `tilt` | The larger of those two changes. This is what the alarm uses |
| `base=waiting` | Hold the model still. It needs about 5 quiet seconds |
| `base=set` | Baseline is stored |
| `wifi=1` | Joined the hotspot |
| `http=200` | Firebase accepted the last write |
| `rain=na` | This is node 1. It has no rain plate |
| `rain=wet` or `rain=dry` | Node 2 only |

All three LEDs flash once at startup. After that, green is normal, yellow is caution, red is warning. If Wi-Fi drops, the active LED blinks, but the colour decision still happens on the board. Node 2 beeps while it is in warning.

Starting limits, until someone changes them on the dashboard:

| Check | Yellow | Red |
|---|---|---|
| Tilt change from baseline | 5 degrees | 10 degrees |
| Relative moisture | 60 percent | 80 percent |
| Rain plate on node 2 | Wet | Used together with high moisture |
| No packet | — | Dashboard shows Offline after 15 seconds |

These are experimental prototype limits, not mine-safety limits.

---

## 11. Run the dashboard on the laptop

1. Join the laptop to the same hotspot as the boards.
2. Open a terminal in the `dashboard` folder:

```
cd C:\path\to\slope_guard_codebase_v1_0\dashboard
npm install
npm run dev
```

3. `npm install` is required once on this laptop. It downloads the website libraries. They are not in the zip.
4. Open the local address the terminal prints, usually `http://localhost:5173`.
5. If the page says **Add the Firebase keys**, `.env` is empty or the dev server was started before you saved `.env`. Fill `.env`, stop the server with Ctrl+C, and run `npm run dev` again.

When both boards are on, the page shows two cards, a slope sketch, two charts, an alert list, and the threshold form. The sentence **Monitoring prototype, not a mine-safety system** stays on the page. Leave it there.

**Set baseline** on a card tells that node to measure a new zero. Hold the model still for about 5 seconds after you press it.

A phone on the same hotspot can open `http://<laptop-ip>:5173`. The terminal prints the network address because the dev server listens on the hotspot.

---

## 12. Put the dashboard on Vercel

Do this after the local page is showing live numbers. The free Hobby plan is enough.

1. Sign in at https://vercel.com with the same GitHub account.
2. Add New → Project → import `slope-guard-360`.
3. Set **Root Directory** to `dashboard`. Framework should detect Vite.
4. Open Environment Variables and add the same five names that are in `dashboard\.env`:

   - `VITE_FIREBASE_API_KEY`
   - `VITE_FIREBASE_AUTH_DOMAIN`
   - `VITE_FIREBASE_DATABASE_URL`
   - `VITE_FIREBASE_PROJECT_ID`
   - `VITE_FIREBASE_APP_ID`

5. Deploy.
6. Open the `*.vercel.app` link. It should show the same live nodes.

Every later `git push` to `main` redeploys the site. The ESP32 firmware is not deployed by Vercel. Firmware changes are still uploaded from the Arduino IDE.

---

## 13. Demo sequence

Both boards and the laptop are on the phone hotspot. The dashboard is open. The model is still, and both cards are green (`base=set` already happened).

1. Both nodes online.
2. Tilt only bench A, about 5 degrees, then about 10. Node 1 goes yellow, then red. The alert names node 1 and tilt. Node 2 stays normal.
3. Wet the soil at bench B. Moisture rises on node 2 only.
4. Put drops on the rain plate. Node 2 can reach warning, and the buzzer sounds.
5. Unplug node 1. Within about 15 seconds that card says Offline. Node 2 keeps updating.

Record one screen video of this sequence as a backup if the phone has no data on the viva day.

---

## 14. What not to put on GitHub

Never commit or re-zip these after they exist:

- `firmware\secrets.h`
- `dashboard\.env`
- `dashboard\node_modules\`
- `dashboard\dist\`

Safe to commit: `secrets.example.h`, `.env.example`, `.gitignore`, the sketches, the dashboard source, and `firebase\database.rules.json`.

Before every commit, run `git status` and look for those secret names.

---

## 15. If something fails

| What you see | What to check |
|---|---|
| No COM port | Data USB cable, not a charge-only cable |
| MPU6050 not found | SDA is GPIO 21, SCL is GPIO 22, power is 3V3, grounds are joined |
| Moisture numbers jump around | Sensor and ESP32 do not share GND, or the signal is not on GPIO 34 |
| Will not join Wi-Fi | Hotspot is 5 GHz, or the password has a symbol, or `secrets.h` has the old example password |
| `http=` is not 200 | `FIREBASE_HOST`, and the Rules tab still has `database.rules.json` |
| Dashboard says it cannot read Firebase | `.env` values, then restart `npm run dev` |
| Dashboard stays empty while serial says `http=200` | You are looking at a different Firebase project than the one in `secrets.h` |
| Data stops weeks later | Rules were left in test mode and expired |
| LEDs flicker on the limit | The value is sitting on 5 degrees or 60 percent. Move it clearly past the limit |
| Wrong node on the dashboard | `config.h` was not changed back, and the serial banner shows the other id |
