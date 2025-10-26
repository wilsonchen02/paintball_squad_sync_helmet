#include "IMU_LIS2MDL.h"

IMU_LIS2MDL::IMU_LIS2MDL(uint8_t sdaPin, uint8_t sclPin, float yawOffset)
  : sdaPin(sdaPin), sclPin(sclPin), yawOffset(yawOffset), mag(12345),
    x_min(0), x_max(0), y_min(0), y_max(0) {}

bool IMU_LIS2MDL::begin() {
  Wire.begin(sdaPin, sclPin);
  if (!mag.begin()) {
    Serial.println("Ooops, no LIS2MDL detected ... Check your wiring!");
    return false;
  }
  Serial.println("LIS2MDL initialized successfully.");
  return true;
}

void IMU_LIS2MDL::setCalibration(float xMin, float xMax, float yMin, float yMax) {
  x_min = xMin;
  x_max = xMax;
  y_min = yMin;
  y_max = yMax;
}

float IMU_LIS2MDL::getHeading() const {
  sensors_event_t event;
  mag.getEvent(&event);

  constexpr float Pi = 3.14159;

  // --- Apply calibration offsets ---
  float x_offset = (x_max + x_min) / 2.0;
  float y_offset = (y_max + y_min) / 2.0;

  float x = event.magnetic.x - x_offset;
  float y = event.magnetic.y - y_offset;

  // --- Calculate heading in degrees ---
  float heading = atan2(y, x) * 180.0 / Pi;

  // --- Normalize to -180 to +180 ---
  if (heading > 180)
    heading -= 360;
  else if (heading < -180)
    heading += 360;

  // --- Apply yaw offset ---
  heading -= yawOffset;

  // --- Wrap again after offset ---
  if (heading > 180)
    heading -= 360;
  else if (heading < -180)
    heading += 360;

  return heading;
}
