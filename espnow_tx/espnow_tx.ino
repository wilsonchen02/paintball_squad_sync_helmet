// /*
//   ESP-NOW Demo - Transmit
//   esp-now-demo-xmit.ino
//   Sends data to Responder
  
//   DroneBot Workshop 2022
//   https://dronebotworkshop.com
// */

// // Include Libraries
// #include <esp_now.h>
// #include <WiFi.h>

// // Variables for test data
// int int_value;
// float float_value;
// bool bool_value = true;

// // MAC Address of responder - edit as required
// uint8_t broadcastAddress[] = {0xEC, 0xDA, 0x3B, 0x8E, 0xD8, 0xDC};

// // Define a data structure
// typedef struct struct_message {
//   char a[32];
//   int b;
//   float c;
//   bool d;
// } struct_message;

// // Create a structured object
// struct_message myData;

// // Peer info
// esp_now_peer_info_t peerInfo;

// // Callback function called when data is sent
// void OnDataSent(const wifi_tx_info_t *mac, esp_now_send_status_t status) {
//   Serial.print("\r\nLast Packet Send Status:\t");
//   Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
// }

// void setup() {
  
//   // Set up Serial Monitor
//   Serial.begin(115200);
 
//   // Set ESP32 as a Wi-Fi Station
//   WiFi.mode(WIFI_STA);

//   // Initilize ESP-NOW
//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error initializing ESP-NOW");
//     return;
//   }

//   // Register the send callback
//   esp_now_register_send_cb(OnDataSent);
  
//   // Register peer
//   memcpy(peerInfo.peer_addr, broadcastAddress, 6);
//   peerInfo.channel = 0;  
//   peerInfo.encrypt = false;
  
//   // Add peer        
//   if (esp_now_add_peer(&peerInfo) != ESP_OK){
//     Serial.println("Failed to add peer");
//     return;
//   }
// }

// void loop() {

//   // Create test data

//   // Generate a random integer
//   int_value = random(1,20);

//   // Use integer to make a new float
//   float_value = 1.3 * int_value;

//   // Invert the boolean value
//   bool_value = !bool_value;
  
//   // Format structured data
//   strcpy(myData.a, "Welcome to the Workshop!");
//   myData.b = int_value;
//   myData.c = float_value;
//   myData.d = bool_value;
  
//   // Send message via ESP-NOW
//   esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
//   if (result == ESP_OK) {
//     Serial.println("Sending confirmed");
//   }
//   else {
//     Serial.println("Sending error");
//   }
//   delay(2000);
// }

/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <WiFi.h>
#include <esp_wifi.h>

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setup(){
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();

  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
}
 
void loop(){

}
