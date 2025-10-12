#include "Nowcom.h"

// Create object for ESPNOW communication instance
Nowcom mynow;

// Create message struct to send over ESP NOW
message helmet_msg;

// FreeRTOS task queue for handling received messages
QueueHandle_t recv_queue; // TODO: implement the queue stuff

// Tasks to periodically send messages
void TaskSendMAC(void *pvParameters);
void TaskSendLocation(void *pvParameters);
void TaskSendSOS(void *pvParameters);
void TaskSendAlert(void *pvParameters);

// Tasks to respond to received messages (triggered by callback)
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

  // Temporarily set the channel to 0
  mynow.set_channel(0);

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

void TaskReceivePairing(void *pvParameters) {
  // Check team ID to make sure it's your team
  // if (msg_content->team_id == team_id) {
  //   // Add new device to m_peers unordered map
  //   m_peers[recv_info->src_addr] = true;

  //   // Actually add to ESP NOW's peer list
  //   esp_now_peer_info_t * new_peer = malloc(sizeof(esp_now_peer_info_t));
  //   memset(new_peer, 0 sizeof(esp_now_peer_info_t));
  //   new_peer->channel = channel;
  //   new_peer->encrypt = is_encrypted;
  //   memcpy(new_peer->peer_addr, recv_info->src_addr, 6);


  //   if (esp_now_add_peer(new_peer) != ESP_OK) {
  //     Serial.println("Failed to add peer");
  //   }
  // }
}
