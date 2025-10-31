#include "espnow.h"

const uint8_t broadcast_addr[MAC_ADDR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Initialization of static members
uint8_t espnow::m_team_id = 0;
esp_now_peer_info_t espnow::m_broadcast_peer = {0}; // For some strange reasons, it must be static?
QueueHandle_t espnow::m_rx_queue = NULL;
uint8_t espnow::m_last_sender_mac[MAC_ADDR_LENGTH] = {0};

/**
 * @brief Constructor
 * 
 * @param channel Team channel set by button interface
 */
espnow::espnow(uint8_t channel, uint8_t team_id) {
  m_channel = channel;
  m_team_id = team_id;
  m_rx_queue = xQueueCreate(10, sizeof(espnow_rx_packet));
}

/**
 * @brief ESP-NOW initialization
 */
void espnow::espnow_init() {


  WiFi.mode(WIFI_STA);

  // ESP-NOW initialization and register callback function
  while (esp_now_init() != ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  esp_now_register_send_cb(send_data_cb);
  esp_now_register_recv_cb(recv_data_cb);

  // Broadcast initialization
  m_broadcast_peer.channel = m_channel;
  m_broadcast_peer.encrypt = BROADCAST_ENCRYPT_FALSE;
  memcpy(m_broadcast_peer.peer_addr, broadcast_addr, MAC_ADDR_LENGTH);
  while (esp_now_add_peer(&m_broadcast_peer) != ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/**
 * @brief Broadcast data in team channel
 * 
 * @param data_type Data type
 * 
 * @param data Data to be sent
 * 
 * @param payload_len Length of data field
 * 
 * @return Operation status
 */
uint8_t espnow::espnow_send_data(message_type data_type, const uint8_t * data, uint8_t payload_len) {
  espnow_packet packet;
  packet.header = PACKET_HEADER;
  packet.length = payload_len + PACKET_DESCRIPTOR_LENGTH;
  packet.team_id = m_team_id;
  packet.data_type = (uint8_t)data_type;
  memset(packet.data, 0, MAX_PAYLOAD_LENGTH); // Initialize the data field
  memcpy(packet.data, data, payload_len);
  packet.checksum = compute_checksum(((uint8_t *)&packet) + PACKET_PREAMBLE_LENGTH, packet.length);

  if (esp_now_send(broadcast_addr, (uint8_t *)&packet, PACKET_LENGTH) == ESP_OK) {
    return ESPNOW_OK;
  }
  else {
    return ESPNOW_ERROR;
  }
}


/**
 * @brief Compute checksum
 * 
 * 0xFF - ((sum of bytes from datatype field to last data byte) & 0xFF)
 * 
 * @return checksum
 */
uint8_t espnow::compute_checksum(const uint8_t * data, uint8_t data_len) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < data_len; i++) {
    sum += data[i];
  }
  uint8_t sum_lowest_byte = sum & 0xFF;
  return 0xFF - sum_lowest_byte;
}

/**
 * @brief Send data callback function
 */
void espnow::send_data_cb(const esp_now_send_info_t * send_info, esp_now_send_status_t status) {

}

/**
 * @brief Recv data callback function 
 * 
 * Store the received packet and trigger the parsing packet task
 * 
 * @param packet Received data
 *
 * @param packet_len Length of received data
 */
void espnow::recv_data_cb(const esp_now_recv_info_t * recv_info, const uint8_t * packet, int packet_len) {
  espnow_rx_packet rx_pkt;
  memcpy(rx_pkt.sender_mac, recv_info->src_addr, MAC_ADDR_LENGTH);   // Copy sender MAC address from recv_info structure
  memcpy(rx_pkt.packet, packet, packet_len);
  rx_pkt.packet_len = packet_len;

  xQueueSend(m_rx_queue, &rx_pkt, pdMS_TO_TICKS(ESPNOW_MAXDELAY));
}
