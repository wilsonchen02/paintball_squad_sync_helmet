#include "../LED/GuidanceStrip.cpp"
#include "../compass_BNO085/IMUHeading.cpp"


#define LED_PIN   6
#define MAX_LED_COUNT   75
#define LED_COUNT   45
#define DEFAULT_BRIGHTNESS   5

#define SDA_PIN 8
#define SCL_PIN 9
#define BNO08X_RESET -1


GuidanceStrip gs(LED_COUNT, LED_PIN, DEFAULT_BRIGHTNESS);

IMUHeading imu(SDA_PIN, SCL_PIN, BNO08X_RESET);

uint8_t mac1[6] = {1, 1, 1, 1, 1, 1};

void setup() {

  //----------- IMU SETUP -----------
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!imu.begin()) {
    Serial.println("Failed to init BNO08x");
    while (1) delay(10);
  }
  Serial.println("BNO08x ready.");


  //----------- GUIDANCE STRIP SETUP -----------
  gs.setLocation(0, 0, 0); //looking straight ahead from 0,0
  gs.setState(STATE_GUIDANCE);
  
  //some example points
  gs.addMate(mac1, 10, 7);
  gs.addMarker(-2, 2, -1);
  gs.addObjective(0, 20);

}

float mate_x = 0;
float mate_speed = 0.01;
int mate_dir = 1;  // 1 = forward, -1 = backward

void loop() {
  //mate walking back and forth
  mate_x += mate_speed * mate_dir;

  if (mate_x >= 20) {
    mate_x = 20;
    mate_dir = -1;
  } else if (mate_x <= -20) {
    mate_x = -20;
    mate_dir = 1;
  }
  
  gs.addMate(mac1, mate_x, 7);
  
  imu.update();
  gs.setLocation(0, 0, imu.getHeading());
  gs.update();
}
