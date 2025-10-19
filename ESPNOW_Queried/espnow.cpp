#include "espnow.h"

// Initialization of static members
espnow * espnow::m_instance = NULL;
SemaphoreHandle_t espnow::m_parse_packet_semaphore = NULL;
uint8_t espnow::m_packet_buffer[PACKET_LENGTH] = {0};
uint8_t espnow::m_packet_len = 0;

uint8_t espnow::m_last_sender_mac[MAC_ADDR_LENGTH] = {0};

QueueHandle_t espnow::m_rx_queue = NULL;


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

  m_rx_queue = xQueueCreate(10, sizeof(espnow_rx_packet));

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
    esp_now_peer_info_t broadcast = {0};    
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

  espnow_rx_packet rx_pkt;

  for (;;) {
    // block on the queue until a new packet arrives (queue is the synch mechanism)
    if (xQueueReceive(m_rx_queue, &rx_pkt, portMAX_DELAY) == pdTRUE) {

      if (rx_pkt.data[0] != PACKET_HEADER) {
        continue;
      }

      uint8_t checksum_computed = compute_checksum(rx_pkt.data + PACKET_PREAMBLE_LENGTH, rx_pkt.data[1]);
      if (rx_pkt.data[rx_pkt.length - 1] != checksum_computed) {
        continue;
      }

      uint8_t payload = rx_pkt.data[1] - PACKET_DESCRIPTOR_LENGTH;
      uint8_t data_type = rx_pkt.data[4];

      uint8_t *data = (uint8_t*)malloc(payload);
      if (!data) continue;

      //debugging output
      // Serial.printf("rx_pkt.length: %d\n", rx_pkt.length);
      // Serial.printf("payload: %d\n", payload);
      // Serial.print("Raw bytes: ");
      // for (int i=0; i<rx_pkt.length; i++) {
      //     Serial.printf("%02X ", rx_pkt.data[i]);
      // }
      // Serial.println();

      // Copy payload out of rx_pkt.data
      memcpy(data, rx_pkt.data + PACKET_PREAMBLE_LENGTH + PACKET_DESCRIPTOR_LENGTH, payload);

    if (payload == sizeof(message)) { //could also check if data_type ID is correct here
          
          message *received_msg = (message *)data;

          Serial.printf("Sender MAC: ");
          for(int i=0; i<6; i++) Serial.printf("%02X:", rx_pkt.sender_mac[i]);
          Serial.println();
          
          Serial.printf("Team ID: %d\n", received_msg->team_id);
          Serial.printf("Msg Type: %u\n", (uint8_t)received_msg->msg_type);
          Serial.printf("X: %f\n", received_msg->x);
          Serial.printf("Y: %f\n", received_msg->y);
          Serial.println("------");
          
        } else {
          Serial.printf("Error: Got message struct data type, but wrong size! Expected %d, Got %d\n", sizeof(message), payload);
        }

      free(data);
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
  espnow_rx_packet rx_pkt;

  memcpy(rx_pkt.data, packet, packet_len);
  rx_pkt.length = packet_len;

  memcpy(rx_pkt.sender_mac, recv_info->src_addr, MAC_ADDR_LENGTH);   // Copy sender MAC address from recv_info structure

  // Tells RTOS whether this ISR will cause a higher-priority task to run right after it ends
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xQueueSendFromISR(m_rx_queue, &rx_pkt, &xHigherPriorityTaskWoken);

  // If a higher-priority task was woken up by the queue send, this forces a context switch immediately after the ISR
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
