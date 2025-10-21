#include "espnow.h"

// Channel 0, Team 10
espnow now(0, 10);

void setup() {
  // put your setup code here, to run once:
  demo_ms1_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
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

/**
 * @brief Task function for sending test data
 */
void demo_ms1_task(void *pvParameters) {
  const TickType_t xPeriod = pdMS_TO_TICKS(500);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
    
    locdata loc;
    loc.lat = 123.45;
    loc.lon = 5678.9;
    now.espnow_send_data(message_type::Location, (uint8_t *)&loc, sizeof(loc));
  }
}
