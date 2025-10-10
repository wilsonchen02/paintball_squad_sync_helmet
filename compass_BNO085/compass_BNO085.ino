#include <Adafruit_BNO08x.h>
#include <Wire.h>
#define BNO08X_RESET -1

//must be these pins for I2C on ESP32S3
#define SDA_PIN 8 
#define SCL_PIN 9

struct euler_t {
  float yaw;
  float pitch;
  float roll;
} ypr;

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

long reportIntervalUs = 20000; // 50Hz

// Vars to hold most recent report values
float yaw, pitch, roll, accuracy;
int acc_status;

void setReports(long report_interval) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ROTATION_VECTOR, report_interval)) {
    Serial.println("Could not enable rotation vector");
  }
}

void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t* ypr, bool degrees = false) {
  float sqr = sq(qr);
  float sqi = sq(qi);
  float sqj = sq(qj);
  float sqk = sq(qk);

  ypr->yaw =  atan2( 2.0 * (qi * qj + qk * qr),  (sqi - sqj - sqk + sqr));
  ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
  ypr->roll = atan2( 2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

  if (degrees) {
    ypr->yaw   *= RAD_TO_DEG;
    ypr->pitch *= RAD_TO_DEG;
    ypr->roll  *= RAD_TO_DEG;
  }
}

void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector, euler_t* ypr, bool degrees = false) {
  quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
}

float getHeading() {
  // This heading is relative to magnetic North. Add the local declination for True North.
  // Returns degrees East of North.
  if (yaw > -90 && yaw < 180) { // Quadrants I,II,IV
    return (90 - yaw);
  } else {                      // Quadrant III
    return -(yaw + 270);
  }
}

void setup(void) {
  //  Serial.begin(115200);
  Serial.begin(230400);
  while (!Serial)
    delay(10);
  Serial.println("Serial started. Try and initialise BNO08x.");

  Wire.begin(SDA_PIN, SCL_PIN); //set up I2C on the ESP32S3
  Wire.setClock(400000);

  // Try to initialize!
  if (!bno08x.begin_I2C(0x4B, &Wire)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  setReports(reportIntervalUs);
}

void loop() {
  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
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

  float currentHeading = getHeading();
  Serial.print("Heading: "); Serial.println(currentHeading); //-180 to 180; 0 corresponds to north
  // Serial.print("  YPR: ");
  // Serial.print(yaw); Serial.print(" ");
  // Serial.print(pitch); Serial.print(" ");
  // Serial.print(roll);
  // Serial.print("  RV Accuracy: ");
  // Serial.print(accuracy);
  // Serial.print("  Status: ");
  // Serial.println(acc_status);
}
