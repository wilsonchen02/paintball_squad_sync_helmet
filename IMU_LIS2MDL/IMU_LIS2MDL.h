#pragma once
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS2MDL.h>
#include <Adafruit_LSM303_Accel.h>

class IMU_LIS2MDL {
public:
  IMU_LIS2MDL(uint8_t sdaPin, uint8_t sclPin, float yawOffset = 0.0);

  bool begin();

  void setCalibration(float xMin, float xMax,
                      float yMin, float yMax,
                      float zMin, float zMax,
                      float rollBiasDeg, float pitchBiasDeg);

  float getHeading() const;

  float getTiltCompensatedHeading();

  void printRawValues();

private:
  uint8_t sdaPin, sclPin;
  float yawOffset;

  float x_min, x_max, y_min, y_max, z_min, z_max;

  float rollBias, pitchBias;

  mutable Adafruit_LIS2MDL mag;
  mutable Adafruit_LSM303_Accel_Unified accel;
};
