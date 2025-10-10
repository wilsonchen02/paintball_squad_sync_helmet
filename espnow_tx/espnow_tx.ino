#include <WiFi.h>
#include <esp_now.h>

// Receiver MAC
uint8_t receiverAddress[] = {0x80, 0xB5, 0x4E, 0xEA, 0x31, 0xF4};

// Message struct (fixed 50-byte payload)
typedef struct {
  char text[50];
} message_t;

message_t msg;

// New callback for ESP-IDF v5.x
void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;  // same channel on both devices
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  strcpy(msg.text, "Hello from ESP-32 A");
}

void loop() {
  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&msg, sizeof(msg));

  if (result == ESP_OK)
    Serial.println("Message sent");
  else
    Serial.println("Error sending message");

  delay(2000);
}
