#include "IMU_SPI.h"

IMU_SPI::IMU_SPI(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t cs, uint8_t reset)
  : miso_pin(miso), mosi_pin(mosi), sck_pin(sck), cs_pin(cs), reset_pin(reset) {}

bool IMU_SPI::begin() {
  SPI.begin(sck_pin, miso_pin, mosi_pin, cs_pin);
}

bool IMU_SPI::read() {
  return rvc.read(&data);
}

float IMU_SPI::getYaw() const {
  return data.yaw;
}

float IMU_SPI::getPitch() const {
  return data.pitch;
}

float IMU_SPI::getRoll() const {
  return data.roll;
}

float IMU_SPI::getAccelX() const {
  return data.x_accel;
}

float IMU_SPI::getAccelY() const {
  return data.y_accel;
}

float IMU_SPI::getAccelZ() const {
  return data.z_accel;
}

void IMU_SPI::setOffset(float offsetDegrees) {
  yawOffset = offsetDegrees;
}

float IMU_SPI::getHeading() const {
  float heading = data.yaw - yawOffset;
  if (heading > 180) heading -= 360;
  if (heading < -180) heading += 360;
  return heading;
}
