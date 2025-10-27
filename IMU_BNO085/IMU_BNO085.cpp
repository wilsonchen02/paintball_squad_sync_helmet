#include "IMU_BNO085.h"
#include <Arduino.h>
#include <math.h>

IMU_BNO085::IMU_BNO085(uint8_t rxPin, uint8_t txPin, int8_t resetPin, float yawOffset)
  : serial(&Serial1),
    bno(resetPin),
    rxPin(rxPin), txPin(txPin), resetPin(resetPin),
    yawOffset(yawOffset),
    yaw(0),
    lastMagX(0), lastMagY(0), lastMagZ(0),
    calibAccel(0), calibGyro(0), calibMag(0) {}

bool IMU_BNO085::begin() {
  // Setup UART
  serial->begin(115200, SERIAL_8N1, rxPin, txPin);
  serial->setRxBufferSize(512); // must be >300
  serial->setTxBufferSize(512);
  delay(50);

  // Reset the BNO if a reset pin is defined
  if (resetPin != -1) {
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(10);
    digitalWrite(resetPin, HIGH);
    delay(100);
  }

  if (!bno.begin_UART(serial)) {
    Serial.println("Failed to communicate with BNO085 over UART");
    return false;
  }
  Serial.println("BNO085 initialized over UART");

  // Enable all reports we may need
  bno.enableReport(SH2_ROTATION_VECTOR);
  bno.enableReport(SH2_GEOMAGNETIC_ROTATION_VECTOR);
  bno.enableReport(SH2_MAGNETIC_FIELD_UNCALIBRATED);
  bno.enableReport(SH2_ACCELEROMETER);
  bno.enableReport(SH2_GYROSCOPE_CALIBRATED);
  bno.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED);

  delay(100);
  return true;
}

bool IMU_BNO085::read() {
  if (!bno.getSensorEvent(&sensorValue))
    return false;

  switch (sensorValue.sensorId) {
    case SH2_ROTATION_VECTOR:
    case SH2_GEOMAGNETIC_ROTATION_VECTOR:
      convertQuaternionToEuler(&sensorValue.un.rotationVector);
      break;

    case SH2_MAGNETIC_FIELD_UNCALIBRATED:
      lastMagX = sensorValue.un.magneticField.x;
      lastMagY = sensorValue.un.magneticField.y;
      lastMagZ = sensorValue.un.magneticField.z;
      break;

    case SH2_ACCELEROMETER:
      calibAccel = sensorValue.status;
      break;
    case SH2_GYROSCOPE_CALIBRATED:
      calibGyro = sensorValue.status;
      break;
    case SH2_MAGNETIC_FIELD_CALIBRATED:
      calibMag = sensorValue.status;
      break;
  }
  return true;
}

void IMU_BNO085::convertQuaternionToEuler(sh2_RotationVectorWAcc_t* q) {
  float q_i = q->i;
  float q_j = q->j;
  float q_k = q->k;
  float q_real = q->real;

  float siny_cosp = 2 * (q_real * q_k + q_i * q_j);
  float cosy_cosp = 1 - 2 * (q_j * q_j + q_k * q_k);
  yaw = atan2(siny_cosp, cosy_cosp) * RAD_TO_DEG;
}

float IMU_BNO085::computeMagHeading(float mx, float my) {
// Apply calibration offsets if available
  float xOffset = (x_max + x_min) / 2.0;
  float yOffset = (y_max + y_min) / 2.0;
  mx -= xOffset;
  my -= yOffset;

  float headingDeg = atan2(my, mx) * RAD_TO_DEG;
  if (headingDeg > 180) headingDeg -= 360;
  if (headingDeg < -180) headingDeg += 360;
  return headingDeg;
}

float IMU_BNO085::getHeading(uint8_t headingMode) {
  float headingDeg = NAN;

  if (headingMode == SH2_MAGNETIC_FIELD_UNCALIBRATED) {
    headingDeg = computeMagHeading(lastMagX, lastMagY);
  } else if (headingMode == SH2_ROTATION_VECTOR || headingMode == SH2_GEOMAGNETIC_ROTATION_VECTOR) {
    headingDeg = yaw;
  }

  // Apply yaw offset and normalize
  headingDeg -= yawOffset;
  if (headingDeg > 180) headingDeg -= 360;
  if (headingDeg < -180) headingDeg += 360;
  return headingDeg;
}

float IMU_BNO085::getAverageHeading(uint8_t headingMode) {
  float heading = getHeading(headingMode);
  if (isnan(heading)) return NAN;

  headingHistory[headingIndex++] = heading;
  if (headingIndex >= NUM_HEADINGS) {
    headingIndex = 0;
    headingFilled = true;
  }

  int count = headingFilled ? NUM_HEADINGS : headingIndex;
  float sum = 0;
  for (int i = 0; i < count; i++) sum += headingHistory[i];
  return sum / count;
}

void IMU_BNO085::setOffset(float offsetDegrees) { 
  yawOffset = offsetDegrees; 
}

void IMU_BNO085::setCalibration(float xMin, float xMax, float yMin, float yMax) {
  x_min = xMin;
  x_max = xMax;
  y_min = yMin;
  y_max = yMax;
}

bool IMU_BNO085::isFullyCalibrated() {
  return (calibAccel == 3 && calibGyro == 3 && calibMag == 3);
}

void IMU_BNO085::printCalibrationStatus() {
  auto statusStr = [](uint8_t s) {
    if (s == 0) return "Unreliable";
    if (s == 1) return "Low";
    if (s == 2) return "Medium";
    if (s == 3) return "High";
    return "Unknown";
  };
  Serial.print(F("[Calib: A=")); Serial.print(statusStr(calibAccel));
  Serial.print(F(" G=")); Serial.print(statusStr(calibGyro));
  Serial.print(F(" M=")); Serial.print(statusStr(calibMag));
  Serial.println(F("]"));
}

void IMU_BNO085::printRawMagnetic() {
  if (!bno.getSensorEvent(&sensorValue)) return;

  if (sensorValue.sensorId == SH2_MAGNETIC_FIELD_UNCALIBRATED) {
    float mx = sensorValue.un.magneticField.x;
    float my = sensorValue.un.magneticField.y;
    float mz = sensorValue.un.magneticField.z;

    Serial.print(F("Magnetic Field (Uncalibrated) "));
    Serial.print(F("X: "));
    Serial.print(mx, 2);
    Serial.print(F("  Y: "));
    Serial.print(my, 2);
    Serial.print(F("  Z: "));
    Serial.println(mz, 2);
  }
}


