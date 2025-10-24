#ifndef IMU_UART_H
#define IMU_UART_H

#include <Arduino.h>
#include "Adafruit_BNO08x_RVC.h"

class IMU_UART {
public:
  IMU_UART(uint8_t rxPin, uint8_t txPin);

  bool begin();
  void calibrate();
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
  uint8_t rxPin;
  uint8_t txPin;
  float yawOffset;

  HardwareSerial* serial;
  Adafruit_BNO08x_RVC rvc;
  BNO08x_RVC_Data data;
};

#endif
