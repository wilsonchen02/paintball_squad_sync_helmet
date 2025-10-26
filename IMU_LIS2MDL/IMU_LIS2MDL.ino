#include "IMU_LIS2MDL.h"

#define SDA_PIN 8
#define SCL_PIN 9

IMU_LIS2MDL imu(SDA_PIN, SCL_PIN, 0);

void setup() {
  Serial.begin(115200);

  if (!imu.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  imu.setCalibration(-5, 76, 26, 111);
}

void loop() {
  float heading = imu.getHeading();
  Serial.print("Heading: ");
  Serial.println(heading);
  delay(100);
}
