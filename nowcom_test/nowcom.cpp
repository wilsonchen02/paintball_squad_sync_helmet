#include "nowcom.h"

void nowcom::send_data_cb(const esp_now_send_info_t * tx_info, esp_now_send_status_t status) {
	
}

void nowcom::recv_data_cb(const esp_now_recv_info_t * recv_info, const uint8_t * data, int len) {
	Serial.println(len);
	for (int i = 0; i < len; i++) {
        Serial.printf("%02X", data[i]);
  }
	Serial.println();

  // Check message type
  uint8_t * msg_id;
  memcpy(msg, data + HEADER_LENGTH, sizeof(data))
  switch () {
    case (Pairing):
      // Check team ID to make sure it's your team
      if (data->team_id == team_id) {
        // Create new peer
        esp_now_peer_info_t new_peer;
        new_peer.channel = channel;
        new_peer.encrypt = false; // Could remove this
        new_peer.peer_addr = recv_info->src_addr;

        // Add new device to m_peers
        m_peers[new_peer] = true;
      }
      break;

    case (Location):
      // Modify marker on LED Strip corresponding to receiving device
      // TODO: 
    case (SOS):

    case (Alert):
  }

  // Set flag for device to true
}

uint8_t nowcom::now_init() {
	// Start Wi-Fi station mode before ESP-NOW
	WiFi.mode(WIFI_STA);
	
	for (int i = 0; i < NOW_MAX_ATTEMPS; i++) {
		if (esp_now_init() == ESP_OK) {
			esp_now_register_send_cb(send_data_cb);
			esp_now_register_recv_cb(recv_data_cb);
			for (int i = 0; i < NOW_MAX_ATTEMPS; i++) {
				if (esp_wifi_get_mac(WIFI_IF_STA, m_self_mac) == ESP_OK) {
					return NOW_OK;
				}
			}
			
			return NOW_ERROR;
		}
	}
	
	return NOW_ERROR;
}

const uint8_t * nowcom::now_get_self_mac() {
	return m_self_mac;
}

uint8_t nowcom::broadcast_init() {
	m_broadcast.channel = BROADCAST_CHANNEL;
  m_broadcast.encrypt = BROADCAST_ENCRYPT;
	
	uint8_t broadcast_addr[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  memcpy(m_broadcast.peer_addr, broadcast_addr, MAC_ADDR_LEN);
	
	for (int i = 0; i < NOW_MAX_ATTEMPS; i++) {
		if (esp_now_add_peer(&m_broadcast) == ESP_OK) {
			return NOW_OK;
		}
	}
	
	return NOW_ERROR;
}

uint8_t nowcom::now_send_msg(uint8_t * addr, uint8_t * data) {
	const uint8_t header[HEADER_LENGTH] = message_HEADER;
	const uint8_t footer[FOOTER_LENGTH] = message_FOOTER;
	const uint8_t data_size = sizeof(data);
	Serial.println(data_size);
	Serial.println(data_size + message_OVERHEAD);
	uint8_t message[data_size + message_OVERHEAD];
	
	memcpy(message, header, HEADER_LENGTH);
	memcpy(message + HEADER_LENGTH, data, data_size);
	memcpy(message + HEADER_LENGTH + data_size, footer, FOOTER_LENGTH);
	for (int i = 0; i < sizeof(message); i++) {
		Serial.printf("%2X", message[i]);
	}
	Serial.println();
	
	for (int i = 0; i < NOW_MAX_ATTEMPS; i++) {
		if (esp_now_send(addr, message, sizeof(message)) == ESP_OK) {
			return NOW_OK;
		}
	}
	
	return NOW_ERROR;
}

uint8_t nowcom::now_recv_msg(const uint8_t * message, uint8_t * data) {
	const uint8_t expected_header[HEADER_LENGTH] = message_HEADER;
    const uint8_t expected_footer[FOOTER_LENGTH] = message_FOOTER;
	uint8_t message_size = sizeof(message);
	uint8_t data_size = message_size - HEADER_LENGTH - FOOTER_LENGTH;
	
	if (memcmp(message, expected_header, HEADER_LENGTH) != 0) {
        return NOW_ERROR;
    }
    if (memcmp(message + HEADER_LENGTH + data_size, expected_footer, FOOTER_LENGTH) != 0) {
        return NOW_ERROR;
    }

    memcpy(data, message + HEADER_LENGTH, data_size);
	return NOW_OK;
}

uint8_t nowcom::broadcast_mac_address() {
	return now_send_msg(m_broadcast.peer_addr, m_self_mac);
}
