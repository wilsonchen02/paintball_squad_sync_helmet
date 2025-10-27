#include "IMU_BNO085.h"

IMU_BNO085 imu(17, 18, 5, 0);  // RX=17, TX=18, RESET=5, offset=18°

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!imu.begin()) {
    Serial.println("BNO085 init failed!");
    while (1);
  }

  Serial.println("BNO085 Ready!");
}

void loop() {
  if (imu.read()) {
    float heading1 = imu.getHeading(SH2_ROTATION_VECTOR);
    float heading2 = imu.getHeading(SH2_GEOMAGNETIC_ROTATION_VECTOR);
    float heading3 = imu.getHeading(SH2_MAGNETIC_FIELD_UNCALIBRATED);

    Serial.println("---------------------------");
    Serial.print("Rot Vec: "); Serial.print(heading1, 1);
    Serial.print("   |   Geo Mag: "); Serial.print(heading2, 1);
    Serial.print("   |  Raw Mag: "); Serial.println(heading3, 1);
    imu.printRawMagnetic();

  }
  delay(100);
}
