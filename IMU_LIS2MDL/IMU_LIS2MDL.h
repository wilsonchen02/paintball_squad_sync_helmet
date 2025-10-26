#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS2MDL.h>

class IMU_LIS2MDL {
public:
  IMU_LIS2MDL(uint8_t sdaPin, uint8_t sclPin, float yawOffset = 0.0);

  bool begin();                     // Initialize the LIS2MDL
  float getHeading() const;         // Returns calibrated heading (yaw corrected with offset)
  void setCalibration(float xMin, float xMax, float yMin, float yMax); // Set calibration values

private:
  uint8_t sdaPin;
  uint8_t sclPin;
  float yawOffset;

  mutable Adafruit_LIS2MDL mag;

  // Calibration values (default to 0 so user must set them)
  float x_min, x_max, y_min, y_max;
};
