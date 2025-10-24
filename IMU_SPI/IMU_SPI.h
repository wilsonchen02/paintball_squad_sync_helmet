#ifndef IMU_SPI_H
#define IMU_SPI_H

#include <Arduino.h>
#include <Adafruit_BNO08x.h>

class IMU_SPI {
public:
  IMU_SPI(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t cs, uint8_t reset);

  bool begin();
  bool read();
  float getYaw() const;
  float getPitch() const;
  float getRoll() const;
  float getAccelX() const;
  float getAccelY() const;
  float getAccelZ() const;

  float getHeading() const;               // returns yaw corrected with offset
  void setOffset(float offsetDegrees);    // allow dynamic calibration

private:
  uint8_t miso_pin;
  uint8_t mosi_pin;
  uint8_t sck_pin;
  uint8_t cs_pin;
  uint8_t reset_pin;
  float yawOffset;
  Adafruit_BNO08x imu;

};

#endif
