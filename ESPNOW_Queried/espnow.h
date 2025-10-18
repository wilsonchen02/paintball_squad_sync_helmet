#ifndef ESPNOW_H
#define ESPNOW_H

#include <WiFi.h>
#include <esp_now.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Operation Status
#define ESPNOW_OK 0
#define ESPNOW_ERROR 1

// Packet Overhead Related
#define PACKET_HEADER 0xEC
#define PACKET_OVERHEAD_LENGTH 6 // Length of overhead, excluding the data field
#define PACKET_PREAMBLE_LENGTH 2 // Length of header and length field
#define PACKET_DESCRIPTOR_LENGTH 3 // Length of sender id and data type
#define PACKET_CHECKSUM_LENGTH 1 // Length of checksum
#define MAX_PAYLOAD_LENGTH 12


#define PACKET_LENGTH (MAX_PAYLOAD_LENGTH + PACKET_OVERHEAD_LENGTH) // Length of packet


// Misc
#define MAC_ADDR_LENGTH 6
#define BROADCAST_ADDR {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define BROADCAST_ENCRYPT_FALSE false

/**
 * @brief ESP-NOW data packet structure
 * 
 * Packet format:
 * [Header][Length][SenderID_H][SenderID_L][DataType][Data][Checksum]
 */
struct espnow_packet {
  uint8_t header; // 0xEC
  uint8_t length; // Number of bytes after this field until checksum
  uint8_t sender_id_h; // Unique 16-bit sender id, high byte
  uint8_t sender_id_l; // Unique 16-bit sender id, low byte
  uint8_t data_type; // Data type
  uint8_t data[MAX_PAYLOAD_LENGTH]; // Data
  uint8_t checksum; // Checksum
};

enum class message_type : unsigned char {
  Location,
  Engaged,
  SOS,
  ClearSOS
};

struct message {
  uint8_t team_id;
  message_type msg_type;
  float x;
  float y;
};

//local packet structure to temporarily hold the received data
struct espnow_rx_packet {
  uint8_t data[PACKET_LENGTH];
  uint8_t length;
  uint8_t sender_mac[MAC_ADDR_LENGTH];
};

class espnow {
public:
  espnow(uint8_t channel);
  uint8_t espnow_init();
  uint8_t espnow_send_data(uint8_t data_type, const uint8_t * data, uint8_t payload);

private:
  uint8_t m_device_id_h;
  uint8_t m_device_id_l;
  uint8_t m_channel;

  // Static
  static espnow * m_instance;
  static SemaphoreHandle_t m_parse_packet_semaphore;
  static uint8_t m_packet_buffer[PACKET_LENGTH]; // Safely store the received data
  static uint8_t m_packet_len; // Length of the received packet

  static uint8_t m_last_sender_mac[MAC_ADDR_LENGTH]; //stores mac address of sender


  static QueueHandle_t m_rx_queue;

  static void task_parse_packet(void* pvParameters);
  static uint8_t compute_checksum(const uint8_t * packet, uint8_t packet_len);

  static void send_data_cb(const esp_now_send_info_t * send_info, esp_now_send_status_t status);
  static void recv_data_cb(const esp_now_recv_info_t * recv_info, const uint8_t * packet, int packet_len);
  
  // Demo function for Milestone 1
  static void espnow_demo_ms1(uint8_t data_type, const uint8_t * data, uint8_t payload, const uint8_t * sender_mac);
};

#endif
