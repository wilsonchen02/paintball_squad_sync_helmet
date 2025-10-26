#include "IMU_LIS2MDL.h"

IMU_LIS2MDL::IMU_LIS2MDL(uint8_t sdaPin, uint8_t sclPin, float yawOffset)
    : sdaPin(sdaPin), sclPin(sclPin), yawOffset(yawOffset),
      mag(12345), accel(54321),
      x_min(0), x_max(0), y_min(0), y_max(0),
      z_min(0), z_max(0), rollBias(0), pitchBias(0) {}

bool IMU_LIS2MDL::begin() {
  Wire.begin(sdaPin, sclPin);

  bool ok = true;

  if (!mag.begin()) {
    Serial.println("Ooops, no LIS2MDL detected ... Check your wiring!");
    ok = false;
  } else {
    Serial.println("LIS2MDL magnetometer initialized.");
  }

  if (!accel.begin()) {
    Serial.println("Ooops, no LSM303 Accel detected ... Check your wiring!");
    ok = false;
  } else {
    Serial.println("LSM303 accelerometer initialized.");
  }

  return ok;
}

void IMU_LIS2MDL::setCalibration(float xMin, float xMax,
                                 float yMin, float yMax,
                                 float zMin, float zMax,
                                 float rollBiasDeg, float pitchBiasDeg) {
  x_min = xMin; x_max = xMax;
  y_min = yMin; y_max = yMax;
  z_min = zMin; z_max = zMax;
  rollBias  = rollBiasDeg  * PI / 180.0;
  pitchBias = pitchBiasDeg * PI / 180.0;
}

float IMU_LIS2MDL::getHeading() const {
  sensors_event_t magEvent;
  mag.getEvent(&magEvent);

  float x_off = (x_max + x_min) / 2.0;
  float y_off = (y_max + y_min) / 2.0;
  float x = magEvent.magnetic.x - x_off;
  float y = magEvent.magnetic.y - y_off;

  float heading = atan2(y, x) * 180.0 / PI;
  if (heading > 180) heading -= 360;
  else if (heading < -180) heading += 360;

  heading -= yawOffset;
  if (heading > 180) heading -= 360;
  else if (heading < -180) heading += 360;

  return heading;
}

float IMU_LIS2MDL::getTiltCompensatedHeading() {
  sensors_event_t accelEvent, magEvent;
  accel.getEvent(&accelEvent);
  mag.getEvent(&magEvent);

  // --- Hard-iron offset ---
  float mx = magEvent.magnetic.x - (x_max + x_min) / 2.0;
  float my = magEvent.magnetic.y - (y_max + y_min) / 2.0;
  float mz = magEvent.magnetic.z - (z_max + z_min) / 2.0;

  float ax = accelEvent.acceleration.x;
  float ay = accelEvent.acceleration.y;
  float az = accelEvent.acceleration.z;

  // --- Roll & pitch ---
  float roll  = atan2(ay, az) - rollBias;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) - pitchBias;

  // --- Tilt compensation ---                                      m_earth = Rx(-pitch) * Ry(-roll) * m_sensor
  float sinR = sin(roll), cosR = cos(roll);
  float sinP = sin(pitch), cosP = cos(pitch);
  float Xh = mx * cosP + mz * sinP;                                 //horizontal X-component of the corrected magnetic field (points N)
  float Yh = mx * sinR * sinP + my * cosR - mz * sinR * cosP;       //horizontal Y-component of the corrected magnetic field (points E)

  // -- calculate heading --
  float heading = atan2(Yh, Xh) * 180.0 / PI;
  if (heading < 0) heading += 360.0;

  heading -= yawOffset;
  // --- Normalize to –180° to +180° ---
  if (heading > 180.0)  heading -= 360.0;
  else if (heading < -180.0) heading += 360.0;

  return heading;
}

void IMU_LIS2MDL::printRawValues() {
  sensors_event_t accelEvent, magEvent;
  accel.getEvent(&accelEvent);
  mag.getEvent(&magEvent);

  float ax = accelEvent.acceleration.x;
  float ay = accelEvent.acceleration.y;
  float az = accelEvent.acceleration.z;

  float roll  = atan2(ay, az) - rollBias;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) - pitchBias;

  Serial.print("Mag X: "); Serial.print(magEvent.magnetic.x, 2);
  Serial.print(" | Y: "); Serial.print(magEvent.magnetic.y, 2);
  Serial.print(" | Z: "); Serial.print(magEvent.magnetic.z, 2);
  Serial.print(" | Roll: "); Serial.print(roll * 180.0 / PI, 1);
  Serial.print("° | Pitch: "); Serial.print(pitch * 180.0 / PI, 1);
  Serial.println("°");
}
