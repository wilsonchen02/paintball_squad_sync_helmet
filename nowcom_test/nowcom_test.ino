#include "nowcom.h"

nowcom mynow;

void TaskSendLocation(void *pvParameters);
void TaskSendMAC(void *pvParameters);

void TaskReceivePairing(void *pvParameters);
void TaskReceiveLocation(void *pvParameters);
void TaskReceiveSOS(void *pvParameters);
void TaskReceiveAlert(void *pvParameters);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  while (mynow.now_init() != NOW_OK) {
    Serial.println("ESP-NOW INIT ERROR!");
  }
  Serial.println("ESP-NOW INIT SUCCESS!");

  Serial.printf("%2X:%2X:%2X:%2X:%2X:%2X\n", mynow.now_get_self_mac()[0], mynow.now_get_self_mac()[1], mynow.now_get_self_mac()[2], 
                mynow.now_get_self_mac()[3], mynow.now_get_self_mac()[4], mynow.now_get_self_mac()[5]);

  // Periodic task to send location of user every 200ms
  xTaskCreate(
    TaskSendLocation,
    "SendLocation",
    128,
    2,
    NULL
  );

  mynow.broadcast_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  mynow.broadcast_mac_address();

  delay(5000);
}

void TaskSendLocation(void *pvParameters) {
  (void) pvParameters;
  for(;;) {
    // TODO: Loop and send messages
  }
}
