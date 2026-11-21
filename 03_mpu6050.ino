// Slope Guard 360 — bench step 3
// Wiring: MPU6050 VCC to 3V3, GND to GND, SDA to GPIO 21, SCL to GPIO 22.
//         Leave AD0 and INT unconnected. Cable under about 25 cm.
// Pass: Serial shows "Found device at 0x68", then roll and pitch numbers
//       that change when you tilt the board.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const int kSda = 21;
const int kScl = 22;

Adafruit_MPU6050 mpu;

void scanI2c() {
  Serial.println("I2C scan:");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  Found device at 0x");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) {
    Serial.println("  No device. Check SDA, SCL, 3V3, and GND.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Slope Guard bench 3 — MPU6050");
  Wire.begin(kSda, kScl);
  scanI2c();
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 begin() failed. Expected address 0x68.");
    while (true) delay(1000);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("Pass: tilt the board and watch roll and pitch change.");
}

void loop() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Accelerometer tilt. Gyro integration would drift on a model that sits still.
  float roll = atan2f(accel.acceleration.y, accel.acceleration.z) * 57.2957795f;
  float pitch = atan2f(-accel.acceleration.x,
                        sqrtf(accel.acceleration.y * accel.acceleration.y +
                              accel.acceleration.z * accel.acceleration.z)) *
                57.2957795f;

  Serial.print("roll=");
  Serial.print(roll, 2);
  Serial.print(" deg   pitch=");
  Serial.print(pitch, 2);
  Serial.println(" deg");
  delay(200);
}
