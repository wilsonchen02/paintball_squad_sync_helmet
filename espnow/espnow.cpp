#include "espnow.h"

// Initialization of static members
espnow * espnow::m_instance = NULL;
SemaphoreHandle_t espnow::m_parse_packet_semaphore = NULL;
uint8_t espnow::m_packet_buffer[PACKET_LENGTH] = {0};
uint8_t espnow::m_packet_len = 0;

// PUBLIC
/**
 * @brief Constructor
 * 
 * @param channel Team channel set by button interface
 */
espnow::espnow(uint8_t channel) {
  m_instance = this;
  m_channel = channel;
  m_parse_packet_semaphore = xSemaphoreCreateBinary();
}

/**
 * @brief ESP-NOW initialization
 * 
 * @return Operation status
 */
uint8_t espnow::espnow_init() {
  // Create a task for parsing received packet
  xTaskCreate(
    task_parse_packet, 
    "task_parse_packet", 
    8192, 
    NULL, 
    1, 
    NULL);

  // Generate unique device ID from MAC address
  uint8_t mac[MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  m_device_id_h = mac[MAC_ADDR_LENGTH - 2];
  m_device_id_l = mac[MAC_ADDR_LENGTH - 1];

  // ESP-NOW initialization and register callback function
  WiFi.mode(WIFI_STA);
  if (esp_now_init() == ESP_OK) {
    esp_now_register_send_cb(send_data_cb);
    esp_now_register_recv_cb(recv_data_cb);

    // Broadcast initialization
    esp_now_peer_info_t broadcast;
    uint8_t broadcast_addr[MAC_ADDR_LENGTH] = BROADCAST_ADDR;
    memcpy(broadcast.peer_addr, broadcast_addr, MAC_ADDR_LENGTH);
    broadcast.channel = m_channel;
    broadcast.encrypt = BROADCAST_ENCRYPT_FALSE;

    if (esp_now_add_peer(&broadcast) == ESP_OK) {
      return ESPNOW_OK;
    }

    return ESPNOW_ERROR;
  }

  return ESPNOW_ERROR;
}

/**
 * @brief Broadcast data in team channel
 * 
 * @param data_type Data type
 * 
 * @param data Data to be sent
 * 
 * @param payload Length of data field
 * 
 * @return Operation status
 */
uint8_t espnow::espnow_send_data(uint8_t data_type, const uint8_t * data, uint8_t payload) {
  espnow_packet packet;
  packet.header = PACKET_HEADER;
  packet.length = payload + PACKET_DESCRIPTOR_LENGTH;
  packet.sender_id_h = m_device_id_h;
  packet.sender_id_l = m_device_id_l;
  packet.data_type = data_type;
  memset(packet.data, 0, MAX_PAYLOAD_LENGTH); // Initialize the data field
  memcpy(packet.data, data, payload);
  packet.checksum = compute_checksum(((uint8_t *)&packet) + PACKET_PREAMBLE_LENGTH, packet.length);

  uint8_t broadcast_addr[MAC_ADDR_LENGTH] = BROADCAST_ADDR;
  if (esp_now_send(broadcast_addr, (uint8_t *)&packet, PACKET_LENGTH) == ESP_OK) {
    return ESPNOW_OK;
  }

  return ESPNOW_ERROR;
}

// PRIVATE FUNCTIONS
/**
 * @brief Task function for parsing packet
 */
void espnow::task_parse_packet(void* pvParameters) {
  for (;;) {
    if (xSemaphoreTake(m_parse_packet_semaphore, portMAX_DELAY) == pdTRUE) {
      // Verify header
      if (m_packet_buffer[0] != PACKET_HEADER) {
        continue;
      }

      // Verify checksum
      uint8_t checksum_computed = compute_checksum(m_packet_buffer + PACKET_PREAMBLE_LENGTH, m_packet_buffer[1]);
      if (m_packet_buffer[m_packet_len - 1] != checksum_computed) {
        continue;
      }

      // Parse packet
      uint8_t payload = m_packet_buffer[1] - PACKET_DESCRIPTOR_LENGTH;
      uint8_t data_type = m_packet_buffer[4];
      uint8_t * data = (uint8_t*)malloc(payload);
      if (data == NULL) {
        continue;
      }
      else {
        memcpy(data, m_packet_buffer + PACKET_PREAMBLE_LENGTH + PACKET_DESCRIPTOR_LENGTH, payload);

        // For Milestone 1 demo only
        espnow_demo_ms1(data_type, data, payload);
        
        if (data != NULL) {
          free(data);
          data = NULL;
        }
      }
    }
  }
}

/**
 * @brief Compute checksum
 * 
 * 0xFF - ((sum of bytes from length field to last data byte) & 0xFF)
 * 
 * @return checksum
 */
uint8_t espnow::compute_checksum(const uint8_t * packet, uint8_t packet_len) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < packet_len; i++) {
    sum += packet[i];
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
  memcpy(m_packet_buffer, packet, packet_len);
  m_packet_len = packet_len;

  xSemaphoreGive(m_parse_packet_semaphore);
}

void espnow::espnow_demo_ms1(uint8_t data_type, const uint8_t * data, uint8_t payload) {
  Serial.begin(115200);
  Serial.printf("DT: %d\n", data_type);
  Serial.printf("D: ");
  for (int i = 0; i < payload; i++) {
    Serial.printf("%2X", data[i]);
  }
  Serial.println();
}
