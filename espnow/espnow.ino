#include "espnow.h"

// TODO: channel should be set by the button interface
espnow now(0);

void demo_ms1_setup();
void task_demo_ms1_loop(void *pvParameters);

void setup() {
  // put your setup code here, to run once:
  demo_ms1_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void demo_ms1_setup() {
  Serial.begin(115200);
  delay(6000);
  Serial.println("====== ESP-NOW DEMO MS1 ======");

  while (now.espnow_init() == ESPNOW_ERROR) {}
  Serial.println("ESP-NOW INIT SUCCESS!");
  delay(4000);

  xTaskCreate(
    task_demo_ms1_loop,
    "task_demo_ms1_loop",
    4096,
    NULL,
    1,
    NULL);
}

void task_demo_ms1_loop(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);
  xLastWakeTime = xTaskGetTickCount();

  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);

    uint8_t sample_data[1] = {0x11};
    now.espnow_send_data(1, sample_data, 1);
  }
}
