#include "IMUHeading.h"

IMUHeading::IMUHeading(int sdaPin, int sclPin, int resetPin)
  : bno08x(resetPin), sdaPin(sdaPin), sclPin(sclPin) {}

bool IMUHeading::begin() {
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(400000);

  if (!bno08x.begin_I2C(0x4B, &Wire)) {
    return false;
  }
  setReports(reportIntervalUs);
  return true;
}

void IMUHeading::setReports(long report_interval) {
  bno08x.enableReport(SH2_ROTATION_VECTOR, report_interval);
}

void IMUHeading::update() {
  if (bno08x.wasReset()) {
    setReports(reportIntervalUs);
  }

  if (bno08x.getSensorEvent(&sensorValue)) {
    acc_status = sensorValue.status;
    quaternionToEulerRV(&sensorValue.un.rotationVector, &ypr, true); // degrees
    yaw = ypr.yaw;
    pitch = ypr.pitch;
    roll = ypr.roll;
    accuracy = sensorValue.un.rotationVector.accuracy;
  }
}

float IMUHeading::getHeading() {
  if (yaw > -90 && yaw < 180) { // Quadrants I, II, IV
    return (90 - yaw);
  } else {                      // Quadrant III
    return -(yaw + 270);
  }
}

float IMUHeading::getYaw()   { return yaw; }
float IMUHeading::getPitch() { return pitch; }
float IMUHeading::getRoll()  { return roll; }

void IMUHeading::quaternionToEuler(float qr, float qi, float qj, float qk, euler_t* ypr, bool degrees) {
  float sqr = sq(qr);
  float sqi = sq(qi);
  float sqj = sq(qj);
  float sqk = sq(qk);

  ypr->yaw = atan2( 2.0 * (qi * qj + qk * qr),  (sqi - sqj - sqk + sqr));
  ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
  ypr->roll = atan2( 2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

  if (degrees) {
    ypr->yaw   *= RAD_TO_DEG;
    ypr->pitch *= RAD_TO_DEG;
    ypr->roll  *= RAD_TO_DEG;
  }
}

void IMUHeading::quaternionToEulerRV(sh2_RotationVectorWAcc_t* rv, euler_t* ypr, bool degrees) {
  quaternionToEuler(rv->real, rv->i, rv->j, rv->k, ypr, degrees);
}
