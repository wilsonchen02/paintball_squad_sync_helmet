#include "IMU_UART.h"

#define CALIBRATE_TIMES 50

IMU_UART::IMU_UART(uint8_t rxPin, uint8_t txPin)
: rxPin(rxPin), txPin(txPin), yawOffset(0), serial(&Serial1) {}

bool IMU_UART::begin() {
  serial->begin(115200, SERIAL_8N1, rxPin, txPin);
  delay(10);
  return rvc.begin(serial);
}

bool IMU_UART::read() {
  return rvc.read(&data);
}

float IMU_UART::getYaw() const {
  return data.yaw;
}

float IMU_UART::getPitch() const {
  return data.pitch;
}

float IMU_UART::getRoll() const {
  return data.roll;
}

float IMU_UART::getAccelX() const {
  return data.x_accel;
}

float IMU_UART::getAccelY() const {
  return data.y_accel;
}

float IMU_UART::getAccelZ() const {
  return data.z_accel;
}

void IMU_UART::setOffset(float offsetDegrees) {
  yawOffset = offsetDegrees;
}

void IMU_UART::calibrate() {
  float offset_sum = 0;
  int i = 0;
  while (i < CALIBRATE_TIMES) {
    if (rvc.read(&data)) {
      offset_sum += data.yaw;
      i++;
      delay(10);
    }
  }
  yawOffset = offset_sum / CALIBRATE_TIMES;
  Serial.println(yawOffset);
}

float IMU_UART::getHeading() const {
  float heading = data.yaw - yawOffset;
  if (heading > 180) heading -= 360;
  if (heading < -180) heading += 360;
  return heading;
}
