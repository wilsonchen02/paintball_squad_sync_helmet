#include "test.h"

// Channel 0, Team 10
espnow now(0, 10);

/**
 * @brief Setup function
 */
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
    
    uint8_t sample_data[2] = {0x11, 0x22};
    now.espnow_send_data(2, sample_data, 2);
  }
}

/**
 * @brief Print the received data
 * 
 * @param data_type
 * 
 * @param data
 * 
 * @param payload_len
 */
void demo_ms1_espnow_print(const uint8_t * data, uint8_t payload_len) {
  Serial.begin(115200);
  for (int i = 0; i < payload_len; i++) {
    Serial.printf("%2X", data[i]);
  }
  Serial.println();
}
