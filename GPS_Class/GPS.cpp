#include "GPS.h"
#include <Arduino.h>

GPS::GPS(int rxPin, int txPin, uint32_t baud)
    : ss(rxPin, txPin), rxPin(rxPin), txPin(txPin), gpsBaud(baud), lastPrint(0) {}

void GPS::begin() {
    Serial.println("Starting GPS connection at default 9600 baud...");
    ss.begin(9600);
    delay(1000);

    Serial.println("Sending command to change GPS baud rate to 57600...");
    ss.print("$PMTK251,57600*2C\r\n");
    delay(250);

    Serial.println("Re-initializing Arduino serial port to 57600 baud...");
    ss.begin(gpsBaud);

    Serial.println("Sending command to set update rate to 10Hz (100ms)...");
    ss.print("$PMTK220,100*2F\r\n");
}

void GPS::update() {
    while (ss.available() > 0) {
        if (gps.encode(ss.read())) {
            if (millis() - lastPrint > 100) {
                printInfo();
                lastPrint = millis();
            }
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
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(gps.location.lng(), 6);
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
