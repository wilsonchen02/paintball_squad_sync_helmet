#ifndef ESPNOW_H
#define ESPNOW_H

#include <WiFi.h>
#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "test.h" // NOTE: Should be removed for release version

// Operation Status
#define ESPNOW_OK 0
#define ESPNOW_ERROR 1

// Packet Related
#define PACKET_HEADER 0xEC
#define PACKET_PREAMBLE_LENGTH 2 // Length of header and length field
#define PACKET_DESCRIPTOR_LENGTH 1 // Length of data type
#define PACKET_CHECKSUM_LENGTH 1 // Length of checksum
#define PACKET_OVERHEAD_LENGTH (PACKET_PREAMBLE_LENGTH + PACKET_DESCRIPTOR_LENGTH + \
                                PACKET_CHECKSUM_LENGTH) // Length of overhead, excluding the data field
#define MAX_PAYLOAD_LENGTH 5
#define PACKET_LENGTH (MAX_PAYLOAD_LENGTH + PACKET_OVERHEAD_LENGTH) // Length of packet

#define PACKET_HEADER_FIELD_INDEX 0
#define PACKET_LENGTH_FIELD_INDEX 1
#define PACKET_DATATYPE_FIELD_INDEX 2
#define PACKET_CHECKSUM_FIELD_INDEX (PACKET_LENGTH - 1) // Last byte of packet

// Misc
#define MAC_ADDR_LENGTH 6
#define BROADCAST_ENCRYPT_FALSE false

extern const uint8_t broadcast_addr[MAC_ADDR_LENGTH];

/**
 * @brief ESP-NOW data packet structure
 * 
 * Packet format:
 * [Header][Length][DataType][Data][Checksum]
 */
struct espnow_packet {
  uint8_t header; // 0xEC
  uint8_t length; // Number of bytes after this field until checksum
  uint8_t data_type; // Data type
  uint8_t data[MAX_PAYLOAD_LENGTH]; // Data/Payload
  uint8_t checksum; // Checksum
};

class espnow {
public:
  espnow(uint8_t channel);
  void espnow_init();
  uint8_t espnow_send_data(uint8_t data_type, const uint8_t * data, uint8_t payload_len);

private:
  uint8_t m_channel;

  // Static
  static esp_now_peer_info_t m_broadcast_peer;
  static SemaphoreHandle_t m_parse_packet_semaphore;
  static SemaphoreHandle_t m_data_process_semaphore;
  static uint8_t m_packet_buffer[PACKET_LENGTH]; // Safely store the received data
  static uint8_t m_packet_len; // Length of the received packet

  static void parse_packet_task(void* pvParameters);
  static void process_data_task(void* pvParameters);
  static uint8_t compute_checksum(const uint8_t * data, uint8_t data_len);

  static void send_data_cb(const esp_now_send_info_t * send_info, esp_now_send_status_t status);
  static void recv_data_cb(const esp_now_recv_info_t * recv_info, const uint8_t * packet, int packet_len);
};

#endif
