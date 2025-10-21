#ifndef ESPNOW_H
#define ESPNOW_H

#include <WiFi.h>
#include <esp_now.h>

// Operation Status
#define ESPNOW_OK 0
#define ESPNOW_ERROR 1

// Packet Related
#define PACKET_HEADER 0xEC
#define PACKET_PREAMBLE_LENGTH 2 // Length of header and length field
#define PACKET_DESCRIPTOR_LENGTH 2 // Length of team id and data type field
#define PACKET_CHECKSUM_LENGTH 1 // Length of checksum
#define PACKET_OVERHEAD_LENGTH (PACKET_PREAMBLE_LENGTH + PACKET_DESCRIPTOR_LENGTH + \
                                PACKET_CHECKSUM_LENGTH) // Length of overhead, excluding the data field
#define MAX_PAYLOAD_LENGTH (2 * sizeof(locdt)) // 2 locdata elements (lat, lon) + 2 redundant bytes
#define PACKET_LENGTH (MAX_PAYLOAD_LENGTH + PACKET_OVERHEAD_LENGTH) // Length of packet

#define PACKET_HEADER_FIELD_INDEX 0
#define PACKET_LENGTH_FIELD_INDEX 1
#define PACKET_TEAMID_FIELD_INDEX 2
#define PACKET_DATATYPE_FIELD_INDEX 3
#define PACKET_CHECKSUM_FIELD_INDEX (PACKET_LENGTH - 1) // Last byte of packet

// Misc
#define MAC_ADDR_LENGTH 6
#define BROADCAST_ENCRYPT_FALSE false
#define ESPNOW_MAXDELAY 100 // Wait at most 100ms when queue is full

typedef float locdt; // Use float for location data type

extern const uint8_t broadcast_addr[MAC_ADDR_LENGTH];

/**
 * @brief ESP-NOW data packet structure
 * 
 * Packet format:
 * [Header][Length][TeamID][DataType][Data][Checksum]
 */
struct espnow_packet {
  uint8_t header; // 0xEC
  uint8_t length; // Number of bytes after this field until checksum
  uint8_t team_id;
  uint8_t data_type; // Message type
  uint8_t data[MAX_PAYLOAD_LENGTH]; // Payload
  uint8_t checksum;
};

enum class message_type : uint8_t {
  Location = 0,
  Engaged = 1,
  SOS = 2,
  ClearSOS = 3
};

// Struct for location data
struct locdata {
  locdt lat;
  locdt lon;
};

//Packet structure to temporarily hold the received data
struct espnow_rx_packet {
  uint8_t sender_mac[MAC_ADDR_LENGTH];
  uint8_t packet[PACKET_LENGTH];
  uint8_t packet_len;
};

class espnow {
public:
  espnow(uint8_t channel, uint8_t team_id);
  void espnow_init();
  uint8_t espnow_send_data(message_type data_type, const uint8_t * data, uint8_t payload_len);

private:
  uint8_t m_channel;

  // Static
  static uint8_t m_team_id;
  static esp_now_peer_info_t m_broadcast_peer;
  static QueueHandle_t m_rx_queue;
  static uint8_t m_last_sender_mac[MAC_ADDR_LENGTH]; //Store mac address of sender

  static void parse_packet_task(void* pvParameters);
  static uint8_t compute_checksum(const uint8_t * data, uint8_t data_len);

  static void send_data_cb(const esp_now_send_info_t * send_info, esp_now_send_status_t status);
  static void recv_data_cb(const esp_now_recv_info_t * recv_info, const uint8_t * packet, int packet_len);
};

#endif
