#ifndef IMUHEADING_H
#define IMUHEADING_H

#include <Adafruit_BNO08x.h>
#include <Wire.h>

// Struct to hold yaw, pitch, roll
struct euler_t {
  float yaw;
  float pitch;
  float roll;
};

class IMUHeading {
public:
  IMUHeading(int sdaPin, int sclPin, int resetPin = -1);

  bool begin();
  void update();
  float getHeading();   // heading in degrees
  float getYaw();       // optional helpers
  float getPitch();
  float getRoll();

private:
  void setReports(long report_interval);
  void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t* ypr, bool degrees = false);
  void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector, euler_t* ypr, bool degrees = false);

  Adafruit_BNO08x bno08x;
  sh2_SensorValue_t sensorValue;

  long reportIntervalUs = 20000; // 50Hz
  int sdaPin, sclPin;
  float yaw, pitch, roll;
  float accuracy;
  int acc_status;
  euler_t ypr;
};

#endif
