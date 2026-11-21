# Firmware

Open these sketches in the Arduino IDE. The sketch folder name and the `.ino` name match, so open the `.ino` file itself.

Board: **ESP32 Dev Module**. Serial Monitor: **115200**.

## Bench sketches

Flash `firmware/bench/01` through `07` on a breadboard before `slope_guard`. Each file’s first comment is the wiring and the pass condition. Do not merge them into one sketch.

| Sketch | Proves |
|---|---|
| `01_esp32_alive` | The board and the USB cable |
| `02_leds` | Green, yellow, red, and the resistors |
| `03_mpu6050` | I2C address 0x68, roll and pitch change when tilted |
| `04_moisture` | Raw ADC on GPIO 34 in air, dry soil, and water |
| `05_rain` | Node 2 only. Raw ADC on GPIO 35, power switched on GPIO 32 |
| `06_buzzer` | Node 2 only |
| `07_wifi` | The board joins the hotspot and rejoins after the hotspot is cycled |

`07_wifi` reads `firmware/secrets.h`. Copy `secrets.example.h` to `secrets.h` first.

Write down the moisture and rain numbers from sketches 04 and 05. You need them below.

## Final sketch

`firmware/slope_guard/slope_guard.ino` is the program both nodes run.

Default `config.h` is node 1:

- `NODE_ID` = `"node01"`
- `HAS_RAIN_SENSOR` = `0`
- `HAS_BUZZER` = `0`

Flash that to the bench A board. Then edit those three lines for node 2 (`node02`, rain `1`, buzzer `1`) and flash the bench B board. Set them back before the next node 1 upload. The Serial banner prints the node id so you can confirm which firmware is on the board.

Also set, per node, after you measure them:

- `MOIST_RAW_DRY` — raw value in dry soil (this maps to 0 percent)
- `MOIST_RAW_WET` — raw value in soaked soil (this maps to 100 percent)
- `RAIN_RAW_WET_CUTOFF` — node 2 only, a value between the dry plate and a plate with drops on it

The raw moisture number goes **down** as the sensor gets wetter. If your soaked reading is higher than your dry reading, the sensor is not in the soil the way the vendor intended; check the probe and the ground wire before swapping the two constants.

`BUZZER_IS_ACTIVE` stays `1` for a module that beeps when you touch its pins to 3V3 and GND. Set it to `0` only for a bare passive disc that just clicks.

## What the serial line means

One line per second:

`node01 roll=… pitch=… dX=… dY=… tilt=… moist=… raw=… rain=… status=… reason=… wifi=… rssi=… base=… http=…`

- `roll` and `pitch` are the sensor angles right now.
- `dX` and `dY` are the change from the baseline. `tilt` is the larger absolute change, and that is what the alarm uses.
- `base=waiting` means the board is still collecting the 5 quiet seconds. Hold the model still.
- `http=200` means Firebase accepted the last publish. Any other code is a failed write.

## Active or passive buzzer

If the buzzer is quiet, or the board resets when it sounds, it is drawing too much current from the pin. Power the buzzer from 5V through an NPN transistor (BC547 or 2N2222): GPIO 33 → 1 kΩ → base, emitter to GND, buzzer between 5V and the collector. Leave `BUZZER_IS_ACTIVE` at `1` in that circuit, because the pin is only switching the transistor.
