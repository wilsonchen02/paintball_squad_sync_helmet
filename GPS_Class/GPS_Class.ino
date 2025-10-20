#include "GPS.h"

// GPS pins (example: MEGA board)
#define RX_PIN 44
#define TX_PIN 43
const uint32_t GPSBaud = 57600;

GPS gps(RX_PIN, TX_PIN, GPSBaud);

void setup() {
  Serial.begin(115200);
  gps.begin();
}

void loop() {
  gps.update();

  double lat = gps.getLatitude();
  double lng = gps.getLongitude();

  gps.printInfo();

  // Serial.print("Lat: ");
  // Serial.print(lat);
  // Serial.print("  Lng: ");
  // Serial.println(lng);

  delay(100);
}