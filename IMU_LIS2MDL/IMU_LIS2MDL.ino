#include "IMU_LIS2MDL.h"

#define SDA_PIN 8
#define SCL_PIN 9  

IMU_LIS2MDL imu(SDA_PIN, SCL_PIN, 0);

void setup() {
  Serial.begin(115200);
  imu.begin();

  imu.setCalibration(
    -45, 88,    // x min/max
    0, 107,     // y min/max
    -30, 60,    // z min/max
    -4, -1       // roll & pitch biases
  );
}

void loop() {
  float TC_heading = imu.getTiltCompensatedHeading();
  Serial.print("TC heading: ");
  Serial.print(TC_heading, 1);

  float heading = imu.getHeading();
  Serial.print("  |   Heading: ");
  Serial.println(heading, 1);

  imu.printRawValues();
  
  Serial.println("--------------------");

  delay(200);
}

