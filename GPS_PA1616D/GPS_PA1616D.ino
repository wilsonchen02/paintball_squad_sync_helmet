#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
//#include <HardwareSerial.h> //HWS alternative

static const int RXPin = 44, TXPin = 43;
static const uint32_t GPSBaud = 57600;

TinyGPSPlus gps;

// serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
//HardwareSerial ss(1); //HWS alternative


void setup()
{
  // Start the Serial Monitor connection
  Serial.begin(115200);

  // STEP 1: Start the GPS connection at its default baud rate
  Serial.println("Starting GPS connection at default 9600 baud...");
  ss.begin(9600);
  //ss.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin); // RX, TX for GPS //HWS alternative
  delay(1000); // Wait for module boot

  // STEP 2: Send the command to the GPS, telling it to switch to 57600 baud
  Serial.println("Sending command to change GPS baud rate to 57600...");
  ss.print("$PMTK251,57600*2C\r\n");

  // STEP 3: Give the module a moment to process the command and change its speed
  delay(250);

  // STEP 4: RE-INITIALIZE our SoftwareSerial port to match the GPS module's NEW speed
  Serial.println("Re-initializing Arduino serial port to 57600 baud...");
  ss.begin(57600);

  // STEP 5: Now that both are at 57600, send the command to set the update rate to 10Hz
  Serial.println("Sending command to set update rate to 10Hz (100ms)...");
  ss.print("$PMTK220,100*2F\r\n");
}

static unsigned long lastPrint = 0;

void loop()
{
  // //displays all raw output
  // while (ss.available()) {
  //   Serial.write(ss.read());
  // }

  while (ss.available() > 0) //is there new data to be read?
    if (gps.encode(ss.read())) //is the sentence correctly encoded?
      if (millis() - lastPrint > 100) { // print once per 100 ms
        displayInfo();
        lastPrint = millis();
      }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
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
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  
  Serial.println();
}
