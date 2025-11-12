#pragma once
#include <Adafruit_BNO08x.h>

class IMU_BNO085 {
public:
  IMU_BNO085(uint8_t rxPin, uint8_t txPin, int8_t resetPin = -1, float yawOffset = 0.0);

  bool begin();
  bool read();

  float getHeading(uint8_t headingMode = SH2_ROTATION_VECTOR);
  float getAverageHeading(uint8_t headingMode = SH2_ROTATION_VECTOR);

  void setOffset(float offsetDegrees);
  void setCurrentHeadingToZero(uint8_t headingMode);

  bool enableReport(uint8_t headingMode);


  void setCalibration(float xMin = 0, float xMax = 0,
                      float yMin = 0, float yMax = 0);
  void printCalibrationStatus();
  bool isFullyCalibrated();

  void printRawMagnetic();

  //float getYaw()  { return yaw; }

private:
  float convertQuaternionToEuler(sh2_RotationVectorWAcc_t* rotationVector);
  float computeMagHeading(float mx, float my);

  HardwareSerial* serial;
  Adafruit_BNO08x bno;
  sh2_SensorValue_t sensorValue;

  uint8_t rxPin, txPin;
  int8_t resetPin;
  float yawOffset;
  float yaw_rot;  
  float yaw_geo;
  float yaw_game;
  
  float x_min, x_max, y_min, y_max;

  float lastMagX, lastMagY, lastMagZ;

  uint8_t calibAccel, calibGyro, calibMag;

  static const int NUM_HEADINGS = 10;
  float headingHistory[NUM_HEADINGS];
  int headingIndex = 0;
  bool headingFilled = false;
};
