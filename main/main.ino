#include "GuidanceStrip.h"
#include "IMU_LIS2MDL.h"
#include "GPS.h"
#include "espnow.h"




//--- Pins ---
#define LED_PIN   6
#define MAX_LED_COUNT   75
#define LED_COUNT   45
#define DEFAULT_BRIGHTNESS   5

#define RX_PIN_GPS 44
#define TX_PIN_GPS 43
const uint32_t GPSBaud = 57600;

#define SDA_PIN_IMU 8
#define SCL_PIN_IMU 9  

#define BUTTON_PIN_1 35
#define BUTTON_PIN_2 36
#define BUTTON_PIN_3 37
#define BUTTON_PIN_4 38



//--- IMU Calibration ---
#define X_MIN 69
#define X_MAX 131

#define Y_MIN -210
#define Y_MAX -157

#define Z_MIN -16
#define Z_MAX 84

#define ROLL_BIAS -4
#define PITCH_BIAS 0



GuidanceStrip gs(LED_COUNT, LED_PIN, DEFAULT_BRIGHTNESS);

IMU_LIS2MDL imu(SDA_PIN_IMU, SCL_PIN_IMU, 0);

GPS gps(RX_PIN_GPS, TX_PIN_GPS, GPSBaud);



uint8_t mac1[6] = {1, 1, 1, 1, 1, 1};

float heading = -999;
float latitude = -999;
float longitude = -999;



QueueHandle_t xButtonQueue;


void setup() {

  //----------- IMU SETUP -----------
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting IMU...");

  if (!imu.begin()) {
    Serial.println("Failed to find IMU!");
    while (1) delay(10);
  }
  
  imu.setCalibration(
    X_MIN, X_MAX,               // x min/max
    Y_MIN, Y_MAX,               // y min/max
    Z_MIN, Z_MAX,               // z min/max
    ROLL_BIAS, PITCH_BIAS       // roll & pitch biases
  );

  Serial.println("IMU initialized.");
  Serial.println("---------------------");

  //----------- GUIDANCE STRIP SETUP -----------
  Serial.println("Starting Guidance Strip...");
  gs.setLocation(0, 0, 0); //looking straight ahead from 0,0
  gs.setState(STATE_TEAM_SELECT);
  Serial.println("Guidance Strip initialized.");
  Serial.println("---------------------");
  
  //----------- GPS SETUP -----------
  Serial.println("Starting GPS...");
  gps.begin();
  Serial.println("GPS initialized.");
  Serial.println("---------------------");

  //-------------- BUTTON PIN SETUP --------------
  // pinMode(BUTTON_PIN_1, INPUT);
  // pinMode(BUTTON_PIN_2, INPUT);
  // pinMode(BUTTON_PIN_3, INPUT);
  // pinMode(BUTTON_PIN_4, INPUT);



  // ---------------- FREE RTOS -----------------

  xButtonQueue = xQueueCreate(10, sizeof(int));

  //---- TASKS -----

  xTaskCreate(button_poll_task, "ButtonPoll", 4096, NULL, 2, NULL);
  xTaskCreate(button_handler_task, "ButtonHandler", 4096, NULL, 3, NULL);
  xTaskCreate(update_location, "UpdateLocation", 8192, NULL, 1, NULL);



  gs.addObjective(10,10);

}




// -------------------- BUTTON POLL TASK --------------------
void button_poll_task(void *pvParameters) {
  const TickType_t xPeriod = pdMS_TO_TICKS(50);
  const TickType_t holdThreshold = pdMS_TO_TICKS(1000); // 1 second hold
  TickType_t xLastWakeTime = xTaskGetTickCount();

  static bool lastState[4] = {HIGH, HIGH, HIGH, HIGH};
  static TickType_t pressTime[4] = {0, 0, 0, 0};

  int pins[4] = {BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3, BUTTON_PIN_4};

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);

    for (int i = 0; i < 4; i++) {
      bool state = digitalRead(pins[i]);

      // ---- Button just pressed ----
      if (state == LOW && lastState[i] == HIGH) {
        pressTime[i] = xTaskGetTickCount();
      }

      // ---- Button just released ----
      else if (state == HIGH && lastState[i] == LOW) {
        TickType_t releaseTime = xTaskGetTickCount();
        TickType_t heldTime = releaseTime - pressTime[i];

        int buttonID;

        if (heldTime >= holdThreshold) {
          // Long press
          buttonID = 100 + (i + 1);  // 101,102,103,104 for long presses
        } else {
          // Short press
          buttonID = i + 1;          // 1,2,3,4 for short presses
        }

        xQueueSend(xButtonQueue, &buttonID, 0);
      }

      lastState[i] = state;
    }
  }
}


// -------------------- BUTTON HANDLER TASK --------------------
void button_handler_task(void *pvParameters) {
  int buttonID;
  for (;;) {
    if (xQueueReceive(xButtonQueue, &buttonID, portMAX_DELAY) == pdTRUE) {
      switch (buttonID) {
        case 1:
          Serial.println("Button 1 pressed!");
          gs.handlePhysicalInput(0);
          break;
        case 2:
          Serial.println("Button 2 pressed!");
          gs.handlePhysicalInput(1);
          break;
        case 3:
          Serial.println("Button 3 pressed!");
          gs.handlePhysicalInput(2);
          break;
        case 4:
          Serial.println("Button 4 pressed!");
          gs.handlePhysicalInput(3);
          break;


        case 101:
          Serial.println("Button 1 long press");
          gs.handlePhysicalInput(4);
          break;
        case 102:
          Serial.println("Button 2 long press");
          gs.handlePhysicalInput(5);
          break;
        case 103:
          Serial.println("Button 3 long press");
          gs.handlePhysicalInput(6);
          break;
        case 104:
          Serial.println("Button 4 long press");
          gs.handlePhysicalInput(7);
          break;
      }
    }
  }
}

void update_location(void *pvParameters) {
  const TickType_t xPeriod = pdMS_TO_TICKS(100);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);

    heading = imu.getAverageHeading();
    latitude = gps.getAverageLatitude();
    longitude = gps.getAverageLongitude();
  }
}





void loop() {


  gs.setLocation(longitude, latitude, heading);
  
  // Serial.println(longitude);
  // Serial.println(latitude);
  // Serial.println(heading);
  // Serial.println("-----");


  gs.update();

  //the tree, roughly
  //18:53:21.125 -> Location: 42.2925150000,-83.7154600000  Date/Time: 10/20/2025 22:53:21.00
  // 18:53:21.225 -> Location: 42.2925150000,-83.7154600000  Date/Time: 10/20/2025 22:53:21.10
  // 18:53:21.355 -> Location: 42.2925150000,-83.7154616670  Date/Time: 10/20/2025 22:53:21.20
  // 18:53:21.455 -> Location: 42.2925150000,-83.7154616670  Date/Time: 10/20/2025 22:53:21.30
  // 18:53:21.620 -> Location: 42.2925150000,-83.7154633330  Date/Time: 10/20/2025 22:53:21.50

}
