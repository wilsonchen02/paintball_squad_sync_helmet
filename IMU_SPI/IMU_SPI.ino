// #include "IMU_SPI.h"
// #include <Adafruit_BNO08x.h>
#include <Arduino.h>
#include <SPI.h>

#define MISO_PIN 13
#define MOSI_PIN 11
#define SCK_PIN 12  // Max 3 MHz
#define CS_PIN 10
#define INT_PIN 9
#define BNO08X_RESET 47

// float getHeading() const {
//   float heading = data.yaw - yawOffset;
//   if (heading > 180) heading -= 360;
//   if (heading < -180) heading += 360;
//   return heading;
// }

// TODO: PS

SPIClass SPI1(HSPI);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Begin IMU SPI setup...");
  // Set SPI pins
  HSPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  

    Serial.println("IMU SPI setup failed.")
    while (1) {
      delay(10);  // Hang
    }
  }


}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }


}
