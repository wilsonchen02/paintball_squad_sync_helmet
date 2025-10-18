#include "espnow.h"

// TODO: channel should be set by the button interface
espnow now(0);

void demo_setup();
void task_demo_loop(void *pvParameters);

void setup() {
  demo_setup();
}

void loop() {} //handled by RTOS task

void demo_setup() {
  Serial.begin(115200);
  delay(6000);
  Serial.println("====== ESP-NOW DEMO ======");

  while (now.espnow_init() == ESPNOW_ERROR) {}
  Serial.println("ESP-NOW INIT SUCCESS!");
  delay(4000);

  xTaskCreate(
    task_demo_loop,
    "task_demo_loop",
    4096,
    NULL,
    1,
    NULL);
}

//test code for sending our message struct
void sendMyMessage() {
  message my_msg;
  my_msg.team_id = 10;
  my_msg.msg_type = message_type::Location;
  my_msg.x = 420.69;
  my_msg.y = -67.21;
  
  now.espnow_send_data(
    1,                          //data type ID
    (uint8_t *)&my_msg,         //Pointer to the struct
    sizeof(my_msg)              //The size of the struct (will be 12)
  );
}

void task_demo_loop(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);
  xLastWakeTime = xTaskGetTickCount();

  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);

    sendMyMessage();
  }
}
