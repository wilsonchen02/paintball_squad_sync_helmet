#ifndef NOWCOM_H
#define NOWCOM_H

#include <vector>
#include <unordered_map>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <FreeRTOS.h>

// For Test
#include <Arduino.h>

#define NOW_MAX_ATTEMPS 5
#define NOW_OK 0
#define NOW_ERROR 1

#define MAC_ADDR_LEN 6

#define BROADCAST_CHANNEL 0
#define BROADCAST_ENCRYPT false

#define MESSAGE_HEADER {0xEC}
#define MESSAGE_FOOTER {0xEE}
#define HEADER_LENGTH 1
#define FOOTER_LENGTH 1
#define MESSAGE_OVERHEAD (HEADER_LENGTH + FOOTER_LENGTH)

enum class message_type {
  Pairing,
  Location,
  SOS,
  Alert
};

// Data message struct
struct message {
  uint8_t team_id;
  message_type msg_type;
  float x;
  float y;
};

class nowcom {
private:
  uint8_t team_id;
  uint8_t channel;
	uint8_t * data_buffer;
	uint8_t m_self_mac[MAC_ADDR_LEN];
	esp_now_peer_info_t m_broadcast;  // Broadcast address FF:FF:FF:FF:FF:FF

  // TODO: if a peer switches teams, have a counter that will remove
  // non-communicating devices
  // value: boolean flag (was there communication in the last 30 seconds?)
  // Receiving callback function will set the flag to true, RTOS task will
  // check receiving flag status
  std::unordered_map<esp_now_peer_info_t, bool> m_peers;

  // Callback functions for send/receive
	static void send_data_cb(const esp_now_send_info_t * send_info, esp_now_send_status_t status);
	static void recv_data_cb(const esp_now_recv_info_t * recv_info, const uint8_t * data, int len);

  // Function that checks if device expired
  // Sets last sent flag to false if there was communication in the last 30 seconds
  void drop_unused_peers();
	
public:
	uint8_t now_init();

  // Get own device's MAC address
	const uint8_t * now_get_self_mac();

  // Set game channel (changes wifi channel)
  void set_channel(uint8_t channel);

  // Set player's team ID
  void set_team(uint8_t team);

  // Send/receive
	uint8_t now_send_msg(uint8_t * addr, uint8_t * data);
	uint8_t now_recv_msg(const uint8_t * msg, uint8_t * data);

  // ===== Team functions =====

  // Sends user's location to teammates
  uint8_t send_location();

  // Sends alert to teammates
  uint8_t send_alert();

  // ===== Broadcast functions =====
	uint8_t broadcast_init();
	uint8_t broadcast_mac_address();

  // Sends SOS signal to all players in the game channel
  uint8_t send_sos();
};

#endif
