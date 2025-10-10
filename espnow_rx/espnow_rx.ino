#include <WiFi.h>
#include <esp_now.h>

// Correct callback for ESP32-S3 / IDF v5.x
void onReceive(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  // Fixed-size message buffer
  char message[51];           // match sender’s fixed 50-byte payload
  memcpy(message, incomingData, 50);
  message[50] = '\0';

  Serial.print("Message: ");
  Serial.println(message);

  // Build MAC address once — prevents truncated prints
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
           recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);

  Serial.print("Received from: ");
  Serial.println(macStr);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500); // let USB serial stabilize

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // Receiver runs asynchronously
}
