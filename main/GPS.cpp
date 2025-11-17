#include "GPS.h"
#include <Arduino.h>

GPS::GPS(int rxPin, int txPin, uint32_t baud)
    : ss(rxPin, txPin), rxPin(rxPin), txPin(txPin), gpsBaud(baud), lastPrint(0) {}



void GPS::printGPSResponse(unsigned long timeout_ms) {
    unsigned long start = millis();
    Serial.println(F("Listening for GPS response..."));
    while (millis() - start < timeout_ms) {
        while (ss.available()) {
            char c = ss.read();
            Serial.write(c);
        }
    }
    Serial.println();
}

void GPS::addReading(double lat, double lon) {
    latBuffer[bufferIndex] = lat;
    lonBuffer[bufferIndex] = lon;
    bufferIndex = (bufferIndex + 1) % GPS_AVG_COUNT;

    if (bufferSize < GPS_AVG_COUNT) {
        bufferSize++;
    }
}
double GPS::getAverageLatitude() {
    if (bufferSize == 0) return 444.0; // no valid data
    double sum = 0;
    for (int i = 0; i < bufferSize; i++) {
        sum += latBuffer[i];
    }
    return sum / bufferSize;
}

double GPS::getAverageLongitude() {
    if (bufferSize == 0) return 444.0; // no valid data
    double sum = 0;
    for (int i = 0; i < bufferSize; i++) {
        sum += lonBuffer[i];
    }
    return sum / bufferSize;
}


void GPS::begin() {
    Serial.println(F("Starting GPS connection at default 9600 baud..."));
    ss.begin(9600);
    delay(1000);

    // Change baud rate to 57600
    Serial.println(F("Sending command to change GPS baud rate to 57600..."));
    ss.print("$PMTK251,57600*2C\r\n");
    delay(250);

    Serial.println(F("Re-initializing Arduino serial port to 57600 baud..."));
    ss.begin(gpsBaud);
    delay(250); // Let module settle

    // SBAS requires <= 5 Hz
    Serial.println(F("Setting update rate to 5Hz (200ms)..."));
    ss.print("$PMTK220,200*2C\r\n");
    //printGPSResponse(800);

    // Enable SBAS (WAAS/EGNOS/MSAS)
    Serial.println(F("Enabling SBAS..."));
    ss.print("$PMTK313,1*2E\r\n");
    //printGPSResponse(1000);

    Serial.println(F("GPS initialization complete."));
}

void GPS::update() {
    while (ss.available() > 0) {
        if (gps.encode(ss.read())) {
            // If the GPS location is valid, add it to the rolling buffer
            // Also, GPS location data must have been updated in the last 3 seconds
            if (gps.location.isValid() && gps.location.age() < 3000) {
                addReading(gps.location.lat(), gps.location.lng());
            }
            else {
                // Could not read valid GPS data
                addReading(444.0, 444.0);
            }
            
            // // Print info every 200 ms as before
            // if (millis() - lastPrint > 200) {
            //     Serial.println("---------------");

            //     printInfo();

            //     Serial.print("Average Location: ");
            //     Serial.print(getAverageLatitude(), 10);
            //     Serial.print(", ");
            //     Serial.println(getAverageLongitude(), 10);

            //     lastPrint = millis();
            // }
        }
    }

    if (millis() > 5000 && gps.charsProcessed() < 10) {
        Serial.println(F("No GPS detected: check wiring."));
        while (true);
    }
}

bool GPS::isConnected() const {
    return gps.charsProcessed() > 0;
}

void GPS::printInfo() {
    Serial.print(F("Location: "));
    if (gps.location.isValid()) {
        Serial.print(gps.location.lat(), 10);
        Serial.print(F(","));
        Serial.print(gps.location.lng(), 10);
    } else {
        Serial.print(F("INVALID"));
    }

    Serial.print(F("  Date/Time: "));
    if (gps.date.isValid()) {
        Serial.print(gps.date.month());
        Serial.print(F("/"));
        Serial.print(gps.date.day());
        Serial.print(F("/"));
        Serial.print(gps.date.year());
    } else {
        Serial.print(F("INVALID"));
    }

    Serial.print(F(" "));
    if (gps.time.isValid()) {
        if (gps.time.hour() < 10) Serial.print(F("0"));
        Serial.print(gps.time.hour());
        Serial.print(F(":"));
        if (gps.time.minute() < 10) Serial.print(F("0"));
        Serial.print(gps.time.minute());
        Serial.print(F(":"));
        if (gps.time.second() < 10) Serial.print(F("0"));
        Serial.print(gps.time.second());
        Serial.print(F("."));
        if (gps.time.centisecond() < 10) Serial.print(F("0"));
        Serial.print(gps.time.centisecond());
    } else {
        Serial.print(F("INVALID"));
    }

    Serial.println();
}

double GPS::getLatitude() {
    if (gps.location.isValid()) {
        return gps.location.lat();
    }
    return 444.0;
}

double GPS::getLongitude() {
    if (gps.location.isValid()) {
        return gps.location.lng();
    }
    return 444.0;
}
