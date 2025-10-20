#ifndef GPS_H
#define GPS_H

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

class GPS {
public:
    GPS(int rxPin, int txPin, uint32_t baud = 57600);

    void begin();             // Initialize GPS and set baud/update rate
    void update();            // Should be called repeatedly in loop()
    bool isConnected() const; // Returns true if GPS is receiving data
    void printInfo();         // Print location and time info

    double getLatitude();   // Returns latitude or 444 if invalid
    double getLongitude();  // Returns longitude or 444 if invalid

private:
    TinyGPSPlus gps;
    SoftwareSerial ss;
    int rxPin;
    int txPin;
    uint32_t gpsBaud;
    unsigned long lastPrint;
};

#endif
