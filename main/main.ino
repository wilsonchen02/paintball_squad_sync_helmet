#include "GuidanceStrip.h"
//#include "IMU_LIS2MDL.h"
#include "IMU_BNO085.h"
#include "GPS.h"
#include "espnow.h"




//--- Pins ---
#define LED_PIN   6
#define MAX_LED_COUNT   75
#define LED_COUNT   49
#define DEFAULT_BRIGHTNESS   5

#define RX_PIN_GPS 44
#define TX_PIN_GPS 43
const uint32_t GPSBaud = 57600;

// #define SDA_PIN_IMU 8 //LIS2MDL
// #define SCL_PIN_IMU 9  

#define RX_PIN_IMU 17 //BNO085
#define TX_PIN_IMU 18
#define RESET_PIN_IMU 5


#define BUTTON_PIN_1 36
#define BUTTON_PIN_2 35
#define BUTTON_PIN_3 39
#define BUTTON_PIN_4 37



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

//IMU_LIS2MDL imu(SDA_PIN_IMU, SCL_PIN_IMU, 0);

IMU_BNO085 imu(RX_PIN_IMU, TX_PIN_IMU, RESET_PIN_IMU, 0);

GPS gps(RX_PIN_GPS, TX_PIN_GPS, GPSBaud);

espnow now;


uint8_t mac1[6] = {1, 1, 1, 1, 1, 1};

float heading = -999;
float latitude = -999;
float longitude = -999;

QueueHandle_t m_rx_queue = espnow::m_rx_queue;

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
  
  // imu.setCalibration(
  //   X_MIN, X_MAX,               // x min/max
  //   Y_MIN, Y_MAX,               // y min/max
  //   Z_MIN, Z_MAX,               // z min/max
  //   ROLL_BIAS, PITCH_BIAS       // roll & pitch biases
  // );
//black cord
  // imu.setCalibration(
  //   17, 76,    // x min/max
  //   -318, -265,     // y min/max
  //   136, 251,    // z min/max
  //   0, 0       // roll & pitch biases
  // );

//white cord
  //   imu.setCalibration(
  //   -18, 42,    // x min/max
  //   -9, 67,     // y min/max
  //   -65, 56,    // z min/max
  //   0, 0       // roll & pitch biases
  // );

  Serial.println("IMU initialized.");
  Serial.println("---------------------");

  //----------- GUIDANCE STRIP SETUP -----------
  Serial.println("Starting Guidance Strip...");
  gs.setLocation(0, 0, 0); //looking straight ahead from 0,0
  Serial.println("Guidance Strip initialized.");
  Serial.println("---------------------");
  
  //----------- GPS SETUP -----------
  Serial.println("Starting GPS...");
  gps.begin();
  Serial.println("GPS initialized.");
  Serial.println("---------------------");

  //----------- ESPNOW INIT -----------
  Serial.println("Starting ESPNOW...");
  now.espnow_init(1, 0);
  Serial.println("ESPNOW initialized.");
  Serial.println("---------------------");


  //-------------- BUTTON PIN SETUP --------------
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  pinMode(BUTTON_PIN_4, INPUT_PULLUP);



  // ---------------- FREE RTOS -----------------

  xButtonQueue = xQueueCreate(10, sizeof(int));

  //---- TASKS -----

  xTaskCreate(button_poll_task, "button_poll_task", 4096, NULL, 1, NULL);
  xTaskCreate(button_handler_task, "button_handler_task", 4096, NULL, 5, NULL);
  xTaskCreate(update_location_task, "update_location_task", 8192, NULL, 3, NULL);
  xTaskCreate(parse_packet_task, "parse_packet_task", 8192, NULL, 3, NULL);
  xTaskCreate(send_packet_task, "send_packet_task", 8192, NULL, 3, NULL);

   gs.addObjective(-83.7154600000,42.2925150000);

   //gs.setState(STATE_GUIDANCE);

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
          gs.handlePhysicalInput(2);
          if(gs.getState() == STATE_GUIDANCE) {
            locdata loc;
            loc.lat = 777;
            loc.lon = 777;

            now.espnow_send_data(message_type::Engaged, (uint8_t *)&loc, sizeof(loc));
          }
          break;
        case 2:
          Serial.println("Button 2 pressed!");
          gs.handlePhysicalInput(1);
          break;
        case 3:
          Serial.println("Button 3 pressed!");
          gs.handlePhysicalInput(3);
          break;
        case 4:
          Serial.println("Button 4 pressed!");
          gs.handlePhysicalInput(0);
          if(gs.getState() == STATE_GUIDANCE) {
            now.espnow_deinit();
            now.espnow_init(gs.getGameCode(), gs.getTeamCode());
          }
          break;


        case 101:
          Serial.println("Button 1 long press");
          break;
        case 102:
          Serial.println("Button 2 long press");
          gs.handlePhysicalInput(4);
          break;
        case 103:
          Serial.println("Button 3 long press");
          break;
        case 104:
          Serial.println("Button 4 long press");
          break;
      }
    }
  }
}

// -------------------- UPDATE LOCATION TASK --------------------
void update_location_task(void *pvParameters) {
  const TickType_t xPeriod = pdMS_TO_TICKS(100);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
    // if(imu.read())
    //   heading = imu.getHeading(SH2_GEOMAGNETIC_ROTATION_VECTOR); //for the BNO085
    
    //heading = imu.getAverageHeading(); //for the LIS2MDL
    latitude = gps.getAverageLatitude();
    longitude = gps.getAverageLongitude();

    gs.setLocation(longitude, latitude, heading);
    //Serial.println(longitude, 6);    Serial.println(latitude, 6); 
    //Serial.println(heading, 6);
    Serial.println(heading, 1);



  }
}

//---- PARSE PACKET TASK ---------
void parse_packet_task(void* pvParameters) {
  espnow_rx_packet rx_pkt = {0};

  for (;;) {
    // block on the queue until a new packet arrives (queue is the synch mechanism)
    QueueHandle_t rxq = espnow::get_rx_queue();
    if (rxq == NULL) {
      // handle gracefully, e.g. delay and continue
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }
    if (xQueueReceive(rxq, &rx_pkt, portMAX_DELAY) == pdTRUE) {
      // Verify packet length to avoid potential array index out-of-bounds exception
      if (rx_pkt.packet_len != PACKET_LENGTH) {
        continue;
      }

      // Verify team
      if (rx_pkt.packet[PACKET_TEAMID_FIELD_INDEX] != gs.getTeamCode()) {
        continue;
      }

      // Verify header
      if (rx_pkt.packet[PACKET_HEADER_FIELD_INDEX] != PACKET_HEADER) {
        continue;
      }

      // // Verify checksum
      // uint8_t checksum_computed = compute_checksum(rx_pkt.packet + PACKET_PREAMBLE_LENGTH, 
      //                                             rx_pkt.packet[PACKET_LENGTH_FIELD_INDEX]);
      // if (rx_pkt.packet[PACKET_CHECKSUM_FIELD_INDEX] != checksum_computed) {
      //   continue;
      // }

      // Get data
      uint8_t payload_len = rx_pkt.packet[PACKET_LENGTH_FIELD_INDEX] - PACKET_DESCRIPTOR_LENGTH;
      message_type data_type = (message_type)rx_pkt.packet[PACKET_DATATYPE_FIELD_INDEX];
      uint8_t * data = (uint8_t*)malloc(payload_len);
      if (data == NULL) {
        continue;
      }
      memcpy(data, rx_pkt.packet + PACKET_PREAMBLE_LENGTH + PACKET_DESCRIPTOR_LENGTH, payload_len);

      locdata loc;
      memcpy(&loc, (locdata *)data, payload_len);          
      Serial.println("------------");         
      char macStr[18]; // 6 bytes * 2 chars + 5 colons + null terminator
      sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
              rx_pkt.sender_mac[0], rx_pkt.sender_mac[1], rx_pkt.sender_mac[2],
              rx_pkt.sender_mac[3], rx_pkt.sender_mac[4], rx_pkt.sender_mac[5]);
      Serial.println(macStr);         
      Serial.printf("Team: %d\n", rx_pkt.packet[PACKET_TEAMID_FIELD_INDEX]);
      Serial.printf("%.7f %.7f\n", loc.lat, loc.lon);

      switch (data_type) {
        case message_type::Location:
          gs.addMate(rx_pkt.sender_mac, loc.lon, loc.lat);
          break;
        case message_type::Engaged:
          gs.mateEngaged(rx_pkt.sender_mac);
          break;
        case message_type::SOS:
          break;
        case message_type::ClearSOS:
          break;
        default:
          break;
      }
      
      if (data != NULL) {
        free(data);
        data = NULL;
      }
    }
  }
}

// ----------- SEND PACKET TASK ---------------
void send_packet_task(void *pvParameters) {
  const TickType_t xPeriod = pdMS_TO_TICKS(500);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
    
    locdata loc;
    loc.lat = latitude;
    loc.lon = longitude;
    if(gs.getState() == STATE_GUIDANCE) //TEMP
    now.espnow_send_data(message_type::Location, (uint8_t *)&loc, sizeof(loc));
  }
}

int lastGameCode = 0;
int lastTeamCode = 0;
void loop() {

   if(lastTeamCode != gs.getTeamCode()) {Serial.print("Team Code: "); Serial.println(gs.getTeamCode()); lastTeamCode = gs.getTeamCode();}
   if(lastGameCode != gs.getGameCode()) {Serial.print("Game Code: "); Serial.println(gs.getGameCode()); lastGameCode = gs.getGameCode();}
   
    if(imu.read())
     heading = imu.getHeading(SH2_ROTATION_VECTOR); //for the BNO085
    
    gps.update();

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
