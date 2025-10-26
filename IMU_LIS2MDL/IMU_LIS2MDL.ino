#include "IMU_LIS2MDL.h"

#define SDA_PIN 8
#define SCL_PIN 9  

IMU_LIS2MDL imu(SDA_PIN, SCL_PIN, 0);

void setup() {
  Serial.begin(115200);
  imu.begin();

  imu.setCalibration(
    -45, 41,    // x min/max
    -50, 35,     // y min/max
    -280, -190,    // z min/max
    0, 0       // roll & pitch biases
  );
}

void loop() {
  float TC_heading = imu.getTiltCompensatedHeading();
  Serial.print("TC heading: ");
  Serial.print(TC_heading, 1);

  float heading = imu.getHeading();
  Serial.print("  |   Heading: ");
  Serial.print(heading, 1);

  float avgTCHeading = imu.getAverageHeading();
  Serial.print("  |   Avg: ");
  Serial.println(avgTCHeading, 1);

  imu.printRawValues();
  
  Serial.println("--------------------");

  delay(200);
}

