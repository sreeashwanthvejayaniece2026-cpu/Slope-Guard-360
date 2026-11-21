# Slope Guard 360 spec (v1)

Source of truth for the firmware and the dashboard. Hardware build steps (perfboard, foam model, wedges) are in the team build guide. This file is the behaviour the code must keep.

## Scope

- Two independent ESP32 nodes. Node 1 is bench A. Node 2 is bench B.
- The system reports normal, caution, and warning. It does not predict failure, and it does not score probability.
- The MPU6050 reports the tilt of the sensor on the model, not ground displacement.
- Moisture is relative: dry calibration = 0 percent, soaked calibration = 100 percent.
- The rain plate detects water on its surface. It is not a rain gauge.
- Thresholds are experimental prototype values, not mine-safety limits.
- Free tools only: Arduino IDE, Firebase Spark, Vercel Hobby, GitHub.

## Pins (both nodes)

| Function | GPIO | Node 1 | Node 2 |
|---|---|---|---|
| MPU6050 SDA | 21 | yes | yes |
| MPU6050 SCL | 22 | yes | yes |
| Moisture analog | 34 | yes | yes |
| Rain analog | 35 | no | yes |
| Rain board power | 32 | no | yes |
| Green LED | 25 | yes | yes |
| Yellow LED | 26 | yes | yes |
| Red LED | 27 | yes | yes |
| Buzzer | 33 | no | yes |

Use the GPIO numbers printed on the board. Analog sensors stay on GPIO 34 and 35 (ADC1) so Wi-Fi can stay on. Do not use GPIO 0, 2, 5, 12, or 15 for outputs. MPU6050 AD0 and INT stay unconnected. The rain board DO pin stays unconnected. Every sensor is powered from 3V3, not 5V or VIN.

Leave GPIO 32 low except during a rain reading, so the plate is not powered all the time.

## Levels

| Parameter | Caution | Warning |
|---|---|---|
| Tilt change from baseline | 5 degrees | 10 degrees |
| Relative moisture | 60 percent | 80 percent |
| Rain plate (node 2) | Wet | Used only in the combined rule |
| No packet | — | Dashboard marks the node offline after 15 seconds |

Tilt used for the level is the larger of the absolute roll change and the absolute pitch change.

Status is the worst level of any parameter. Reason is chosen in this order: tilt at warning, else moisture at warning with a wet plate (`moisture+rain`), else moisture at warning, else tilt at caution, else moisture at caution, else a wet plate (`rain`), else `none`.

A level clears only after the value falls 1.5 degrees, or 3 percent moisture, below the limit that set it.

The baseline is the average roll and pitch over 5 quiet seconds. It is taken after power-up, and again when `commands/<nodeId>/rebaseline` changes. While a baseline is being captured, tilt alarms stay off so a moving board does not false-alarm.

Warning logic runs on the ESP32. The dashboard shows the status and reason the node sent. The dashboard itself decides offline, using Firebase server time.

LEDs: green normal, yellow caution, red warning. All three flash once at startup. If Wi-Fi is down, the active LED blinks. Node 2 beeps while its status is warning.

## Firebase

Realtime Database. The ESP32 writes with the REST API over HTTPS. The dashboard listens with the web SDK. There is no backend server and no login.

| Path | Contents | Written by |
|---|---|---|
| `nodes/node01`, `nodes/node02` | Latest reading | ESP32 |
| `history/node01/<slot>`, `history/node02/<slot>` | Last 100 readings. Slot is `0` … `99` | ESP32 |
| `alerts/<pushId>` | One entry when status or reason changes | ESP32 |
| `thresholds` | The five limits | Dashboard |
| `commands/node01`, `commands/node02` | `{ "rebaseline": <counter> }` | Dashboard |

Reading fields: `nodeId`, `tiltX`, `tiltY`, `tilt`, `moisture`, `rawMoisture`, `status`, `reason`, `rssi`, `ts`. Node 2 also sends `rainWet` and `rawRain`. `ts` is the Firebase server timestamp. `status` is `normal`, `caution`, or `warning`. `reason` is `none`, `tilt`, `moisture`, `rain`, or `moisture+rain`.

An alert has `ts`, `nodeId`, `parameter`, `level`.

Node and history writes use PATCH to fixed paths. They never use POST. Alerts use POST so Firebase can assign the id.

Default thresholds: tilt 5 / 10, moisture 60 / 80, offline 15 seconds.

## Firmware timing

- MPU6050 and moisture are read about every 20 ms and averaged.
- Local status, LEDs, buzzer, and the Serial line run once per second.
- One HTTPS publish goes out about every 3 seconds, and only while Wi-Fi is up.
- Thresholds and the rebaseline counter are read about every 10 seconds. A failed read keeps the last good values. Compiled defaults are used until the first good read.
- Network work runs in its own FreeRTOS task so a slow HTTPS call cannot freeze the LEDs.
- The sensor loop feeds the watchdog.
- HTTPS uses `setInsecure()` because this is a prototype. The report must say the certificate is not checked.

Serial Monitor is 115200 baud, one status line per second.

## Dashboard

One dark page, usable on a phone.

- Top bar: system state, nodes online, last packet time, and the sentence “Monitoring prototype, not a mine-safety system”.
- Slope sketch with markers A and B. Colour plus a text label (Normal, Caution, Warning, Offline).
- One card per node: tilt change, moisture, rain (node 2), online or offline, last seen, reason.
- Two charts, tilt and moisture, both nodes on each chart.
- Alert list, newest 50.
- Threshold form, labelled experimental, saved to Firebase.
- Re-baseline button per node. It increments that node’s counter.
- CSV download of history.
- Offline is recomputed every second from each reading’s `ts` and `.info/serverTimeOffset`. A dead node turns offline even when no new data arrives.

## Secrets

Wi-Fi password and Firebase values live in `firmware/secrets.h` and `dashboard/.env`. Neither file is committed. Example files ship in their place.
