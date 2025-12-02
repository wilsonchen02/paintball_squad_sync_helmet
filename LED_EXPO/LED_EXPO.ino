#include "../main/GuidanceStrip.cpp"
#include "../main/IMU_BNO085.cpp"


#define LED_PIN   6
#define MAX_LED_COUNT   75
#define LED_COUNT   24
#define DEFAULT_BRIGHTNESS   50

#define BNO08X_RESET -1
#define RX_PIN_IMU 17 //BNO085
#define TX_PIN_IMU 18
#define RESET_PIN_IMU 5

#define HEADING_MODE SH2_GAME_ROTATION_VECTOR

GuidanceStrip gs(LED_COUNT, LED_PIN, DEFAULT_BRIGHTNESS);

IMU_BNO085 imu(RX_PIN_IMU, TX_PIN_IMU, RESET_PIN_IMU, 180); 

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

  imu.enableReport(HEADING_MODE);

  Serial.println("IMU initialized.");


  //----------- GUIDANCE STRIP SETUP -----------
  gs.setLocation(0, 0, 0); //looking straight ahead from 0,0
  gs.setState(STATE_GUIDANCE);
  
  //some example points
  gs.addMate(mac1, 0.00010, 0.0004);
  gs.addMarker(-0.0002, 0.0002, -0.0001);
  gs.addObjective(0, 0.00020);

}

float mate_x = 0;
float mate_speed = 0.000005;
int mate_dir = 1;  // 1 = forward, -1 = backward
float heading = 0;

// Time interval for changing mate status (in milliseconds)
float mate_time_intervals[3] = {3000, 3000, 6000};
int interval_iterator = 0;
float prev_time = 0;


void loop() {
  //mate walking back and forth
  mate_x += mate_speed * mate_dir;

  if (mate_x >= 0.00080) {
    mate_x = 0.00080;
    mate_dir = -1;
  } else if (mate_x <= -0.00080) {
    mate_x = -0.00080;
    mate_dir = 1;
  }
  
  // Mate moving left and right
  if(interval_iterator != 2) {
    gs.addMate(mac1, mate_x, 0.0004);
  }
  
  if(imu.read())
    heading = imu.getHeading(HEADING_MODE);

  // Get current time
  unsigned long current_time = millis();
  float time_diff = current_time - prev_time;
  if(time_diff > mate_time_intervals[interval_iterator]) {
    if(interval_iterator == 0) {
      gs.mateEngaged(mac1);
      Serial.println("Mate engaged");
    }
    else if(interval_iterator == 1) {
      gs.mateEliminated(mac1);
      Serial.println("Mate eliminated");
    }
    else if(interval_iterator == 2) {
      Serial.println("Mate respawned");
    }
    interval_iterator++;
    interval_iterator %= 3;
    prev_time = current_time;
  }

  // Set user's location
  gs.setLocation(0, 0, heading);
  
  gs.update();

  // Serial.printf("Heading: %.6f\n", heading);
}
