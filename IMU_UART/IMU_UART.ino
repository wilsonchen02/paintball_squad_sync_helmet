#include "IMU_UART.h"

IMU_UART imu(17, 18, 0); // RX, TX, initial yaw offset

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting IMU...");

  if (!imu.begin()) {
    Serial.println("Failed to find BNO08x!");
    while (1) delay(10);
  }

  Serial.println("IMU initialized.");
}

void loop() {
  if (!imu.read()) return;

  // Serial.println();
  // Serial.println(F("---------------------------------------"));
  // Serial.print(F("Yaw: "));   Serial.print(imu.getYaw());
  // Serial.print(F("\tPitch: ")); Serial.print(imu.getPitch());
  // Serial.print(F("\tRoll: "));  Serial.println(imu.getRoll());

  // Serial.print(F("X: ")); Serial.print(imu.getAccelX());
  // Serial.print(F("\tY: ")); Serial.print(imu.getAccelY());
  // Serial.print(F("\tZ: ")); Serial.println(imu.getAccelZ());

  Serial.print(F("Heading: "));
  Serial.println(imu.getHeading());
}
