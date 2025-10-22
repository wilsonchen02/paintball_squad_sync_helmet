//YOU MUST REMOVE THE TASK_CREATE AND DEFINITION FOR RECEIVING FROM ESPNOW.CPP (it is already placed in here)

#include "../LED/GuidanceStrip.cpp"
#include "../IMU_UART/IMU_UART.cpp"
#include "../GPS_Class/GPS.cpp"
#include "../espnow/espnow.cpp"



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
QueueHandle_t m_rx_queue = espnow::m_rx_queue;

#define YAW_OFFSET 148


GuidanceStrip gs(LED_COUNT, LED_PIN, DEFAULT_BRIGHTNESS);

IMU_UART imu(17, 18, YAW_OFFSET); // RX, TX, initial yaw offset

GPS gps(RX_PIN, TX_PIN, GPSBaud);

espnow now(0, 10);
int m_team_id = 10;

uint8_t mac1[6] = {1,0,0,0,0,0};


void setup() {
  device1_setup();
}

float heading = 0;

void loop() {
  device1_loop();
}

unsigned long lastPrintTime = 0;
void device1_loop() {
  if(imu.read()) {
    heading = imu.getHeading();
  }
  gs.setLocation(-83.7354050, 42.2746048, heading);

// ------------
// 80:B5:4E:EA:31:F4
// Team: 10
// 42.2746048 -83.7354050
//     My Heading: 3.04
// ------------
// 80:B5:4E:EA:31:F4
// Team: 10
// 42.2746048 -83.7354050
//     My Heading: 3.04
// ------------
// Team: 10
// 42.2746086 -83.7354050


  //print heading every 500 ms
  unsigned long currentMillis = millis();
  if (currentMillis - lastPrintTime >= 500) {
    lastPrintTime = currentMillis;
    Serial.begin(115200); // Ensure serial is started
    Serial.print("    My Heading: ");
    Serial.println(heading);
  }


  gs.update();
}

void device2_loop() {
  gps.update();
    My Heading: 151.41
------------
80:B5:4E:EA:31:F4
Team: 10
42.2744751 -83.7355499
    My Heading: 151.43
------------
80:B5:4E:EA:31:F4
Team: 10
42.2744751 -83.7355499
    My Heading: 151.41
}

void device1_setup() {
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



  //--espnow setup and receive task--

  Serial.begin(115200);
  delay(5000);
  Serial.println("====== ESP-NOW DEMO MS1 ======");

  now.espnow_init();
  Serial.println("ESP-NOW INIT SUCCESS!");
  delay(1000);


    // Create a task for parsing received packet
  xTaskCreate(
    parse_packet_task, 
    "parse_packet_task", 
    8192, 
    NULL, 
    1, 
    NULL);
  
}
void device2_setup() {
  
  //----------- GPS SETUP -----------
  gps.begin();

  // ---------- ESPNOW SETUP --------
  demo_ms1_setup();
}



void demo_ms1_setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("====== ESP-NOW DEMO MS1 ======");

  now.espnow_init();
  Serial.println("ESP-NOW INIT SUCCESS!");
  delay(1000);

  xTaskCreate(
    demo_ms1_task,
    "demo_ms1_task",
    2048,
    NULL,
    1,
    NULL);
}

void demo_ms1_task(void *pvParameters) {
  const TickType_t xPeriod = pdMS_TO_TICKS(500);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
    
    locdata loc;
    loc.lat = gps.getLatitude();
    loc.lon = gps.getLongitude();
    now.espnow_send_data(message_type::Location, (uint8_t *)&loc, 2*32/8);
  }
}

/**
 * @brief Task function for parsing packet
 */
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
      if (rx_pkt.packet[PACKET_TEAMID_FIELD_INDEX] != m_team_id) {
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

      switch (data_type) {
        case message_type::Location:
          // === For test only ===
          Serial.begin(115200);
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
          gs.addMate(mac1, loc.lon, loc.lat);
          break;
        case message_type::Engaged:
          // === For test only ===
          for (int i = 0; i < payload_len; i++) {
            Serial.printf("%2X", data[i]);
          }
          Serial.println();
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


