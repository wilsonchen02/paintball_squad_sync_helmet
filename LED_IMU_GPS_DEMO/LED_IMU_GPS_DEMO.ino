#include "../LED/GuidanceStrip.cpp"
#include "../IMU_UART/IMU_UART.cpp"
#include "../GPS_Class/GPS.cpp"



#define LED_PIN   6
#define MAX_LED_COUNT   75
#define LED_COUNT   45
#define DEFAULT_BRIGHTNESS   5

#define SDA_PIN 8
#define SCL_PIN 9
#define BNO08X_RESET -1

#define RX_PIN 44
#define TX_PIN 43
const uint32_t GPSBaud = 57600;



GuidanceStrip gs(LED_COUNT, LED_PIN, DEFAULT_BRIGHTNESS);

IMU_UART imu(17, 18, 0); // RX, TX, initial yaw offset

GPS gps(RX_PIN, TX_PIN, GPSBaud);

uint8_t mac1[6] = {1, 1, 1, 1, 1, 1};

void setup() {

  //----------- IMU SETUP -----------
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting IMU...");

  if (!imu.begin()) {
    Serial.println("Failed to find BNO08x!");
    while (1) delay(10);
  }

  Serial.println("IMU initialized.");

  //----------- GUIDANCE STRIP SETUP -----------
  gs.setLocation(0, 0, 0); //looking straight ahead from 0,0
  gs.setState(STATE_GUIDANCE);
  
  //some example points
  //gs.addMate(mac1, 10, 7);
  //gs.addMarker(-2, 2, -1);
  gs.addObjective(-83.7154633330, 42.2925150000);

  
  //----------- GPS SETUP -----------

  gps.begin();


}

float mate_x = 0;
float mate_speed = 0.01;
int mate_dir = 1;  // 1 = forward, -1 = backward
float heading = 0;

void loop() {
  //mate walking back and forth
  // mate_x += mate_speed * mate_dir;

  // if (mate_x >= 20) {
  //   mate_x = 20;
  //   mate_dir = -1;
  // } else if (mate_x <= -20) {
  //   mate_x = -20;
  //   mate_dir = 1;
  // }
  
  //gs.addMate(mac1, mate_x, 7);

  if(imu.read())
    heading = imu.getHeading();

  gps.update();
  gs.setLocation(0,0, heading);
  gs.setLocation(gps.getLongitude(), gps.getLatitude(), heading);
  gs.update();

  //the tree, roughly
  //18:53:21.125 -> Location: 42.2925150000,-83.7154600000  Date/Time: 10/20/2025 22:53:21.00
  // 18:53:21.225 -> Location: 42.2925150000,-83.7154600000  Date/Time: 10/20/2025 22:53:21.10
  // 18:53:21.355 -> Location: 42.2925150000,-83.7154616670  Date/Time: 10/20/2025 22:53:21.20
  // 18:53:21.455 -> Location: 42.2925150000,-83.7154616670  Date/Time: 10/20/2025 22:53:21.30
  // 18:53:21.620 -> Location: 42.2925150000,-83.7154633330  Date/Time: 10/20/2025 22:53:21.50

}
