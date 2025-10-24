#include "IMU_UART.h"

IMU_UART imu(17, 18); // RX, TX, initial yaw offset

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting IMU...");

  while (!imu.begin()) {
    Serial.println("Failed to find BNO08x!");
    delay(10);
  }

  delay(3000);
  imu.calibrate();

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

  Serial.printf("Heading: %.2f\n", imu.getHeading());
  delay(10);
}
