#include "IMUHeading.h"

#define SDA_PIN 8
#define SCL_PIN 9
#define BNO08X_RESET -1

IMUHeading imu(SDA_PIN, SCL_PIN, BNO08X_RESET);

void setup() {
  Serial.begin(230400);
  while (!Serial) delay(10);

  if (!imu.begin()) {
    Serial.println("Failed to init BNO08x");
    while (1) delay(10);
  }
  Serial.println("BNO08x ready.");
}

void loop() {
  imu.update();
  float heading = imu.getHeading();
  Serial.print("Heading: ");
  Serial.println(heading);
  delay(20);  // 50Hz
}
