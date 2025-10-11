#include "Nowcom.h"

// Create object for ESPNOW communication instance
Nowcom mynow;

// Create message struct to send over ESP NOW
message helmet_msg;

void TaskSendLocation(void *pvParameters);
void TaskSendMAC(void *pvParameters);

void TaskReceivePairing(void *pvParameters);
void TaskReceiveLocation(void *pvParameters);
void TaskReceiveSOS(void *pvParameters);
void TaskReceiveAlert(void *pvParameters);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  // Initialize ESP NOW and check status
  while (mynow.now_init() != NOW_OK) {
    Serial.println("ESP-NOW INIT ERROR!");
  }
  Serial.println("ESP-NOW INIT SUCCESS!");

  // Print this device's MAC address
  Serial.printf("%2X:%2X:%2X:%2X:%2X:%2X\n", mynow.get_self_mac()[0], mynow.get_self_mac()[1], mynow.get_self_mac()[2], 
                mynow.get_self_mac()[3], mynow.get_self_mac()[4], mynow.get_self_mac()[5]);

  // Periodic task to send location of user every 200ms
  // xTaskCreate(
  //   TaskSendLocation,
  //   "SendLocation",
  //   128,
  //   2,
  //   NULL
  // );

  // Sends out MAC address to everyone in channel
  mynow.broadcast_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  mynow.broadcast_mac_address();

  // Print peer list mac addresses (so we know we connected properly)
  mynow.print_peer_mac_addresses();

  delay(5000);
}

void TaskSendLocation(void *pvParameters) {
  (void) pvParameters;
  for(;;) {
    // TODO: Loop and send messages
  }
}
