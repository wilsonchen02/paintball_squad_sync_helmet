#include "espnow.h"

const uint8_t broadcast_addr[MAC_ADDR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Initialization of static members
esp_now_peer_info_t espnow::m_broadcast_peer; // For some strange reasons, it must be static
SemaphoreHandle_t espnow::m_parse_packet_semaphore;
SemaphoreHandle_t espnow::m_data_process_semaphore;
uint8_t espnow::m_packet_buffer[PACKET_LENGTH] = {0};
uint8_t espnow::m_packet_len = 0;

/**
 * @brief Constructor
 * 
 * @param channel Team channel set by button interface
 */
espnow::espnow(uint8_t channel) {
  m_channel = channel;
  m_parse_packet_semaphore = xSemaphoreCreateBinary();
  m_data_process_semaphore = xSemaphoreCreateBinary();
}

/**
 * @brief ESP-NOW initialization
 */
void espnow::espnow_init() {
  // Create a task for parsing received packet
  xTaskCreate(
    parse_packet_task, 
    "parse_packet_task", 
    8192, 
    NULL, 
    1, 
    NULL);
  
  xTaskCreate(
    process_data_task, 
    "process_data_task", 
    4096, 
    NULL, 
    2, 
    NULL);

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
uint8_t espnow::espnow_send_data(uint8_t data_type, const uint8_t * data, uint8_t payload_len) {
  espnow_packet packet;
  packet.header = PACKET_HEADER;
  packet.length = payload_len + PACKET_DESCRIPTOR_LENGTH;
  packet.data_type = data_type;
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
 * @brief Task function for parsing packet
 */
void espnow::parse_packet_task(void* pvParameters) {
  for (;;) {
    if (xSemaphoreTake(m_parse_packet_semaphore, portMAX_DELAY) == pdTRUE) {
      // Verify packet length to avoid potential array index out-of-bounds exception
      if(m_packet_len != PACKET_LENGTH) {
        continue;
      }

      // Verify header
      if (m_packet_buffer[PACKET_HEADER_FIELD_INDEX] != PACKET_HEADER) {
        continue;
      }

      // Verify checksum
      uint8_t checksum_computed = compute_checksum(m_packet_buffer + PACKET_PREAMBLE_LENGTH, 
                                                  m_packet_buffer[PACKET_LENGTH_FIELD_INDEX]);
      if (m_packet_buffer[PACKET_CHECKSUM_FIELD_INDEX] != checksum_computed) {
        continue;
      }

      xSemaphoreGive(m_data_process_semaphore);
    }
  }
}

/**
 * @brief Task function for processing data
 */
void espnow::process_data_task(void* pvParameters) {
  for (;;) {
    if (xSemaphoreTake(m_data_process_semaphore, portMAX_DELAY) == pdTRUE) {
      // Get data
      uint8_t payload_len = m_packet_buffer[PACKET_LENGTH_FIELD_INDEX] - PACKET_DESCRIPTOR_LENGTH;
      uint8_t data_type = m_packet_buffer[PACKET_DATATYPE_FIELD_INDEX];
      uint8_t * data = (uint8_t*)malloc(payload_len);
      if (data == NULL) {
        continue;
      }      
      memcpy(data, m_packet_buffer + PACKET_PREAMBLE_LENGTH + PACKET_DESCRIPTOR_LENGTH, payload_len);

      switch (data_type) {
        case 0x01:
          // For test only
          demo_ms1_espnow_print(data_type + 9, data, payload_len);
          break;
        case 0x02:
          // For test only
          demo_ms1_espnow_print(data_type, data, payload_len);
          break;
        default:
          break;
      }
      
      if (data != NULL) {
        free(data);
        data = NULL;
      }
    }
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
  memcpy(m_packet_buffer, packet, packet_len);
  m_packet_len = packet_len;

  xSemaphoreGive(m_parse_packet_semaphore);
}
