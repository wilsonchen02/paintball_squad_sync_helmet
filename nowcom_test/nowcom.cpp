#include "Nowcom.h"

// NOTE: keep callback functions short and 
void Nowcom::send_data_cb(const esp_now_send_info_t * tx_info, esp_now_send_status_t status) {
	// TODO, doesn't have to do anything for now
}

void Nowcom::recv_data_cb(const esp_now_recv_info_t * recv_info, const uint8_t * data, int len) {
  // Print the sender's MAC address to serial
	Serial.println(len);
  Serial.printf("Receiving data from: ");
	for (int i = 0; i < len; i++) {
    Serial.printf("%02X", data[i]);
  }
	Serial.println();

  uint8_t * extracted_data;
  const message * msg_content;

  // Verify header + footer of message and extract data
  const uint8_t expected_header[HEADER_LENGTH] = MESSAGE_HEADER;
  const uint8_t expected_footer[FOOTER_LENGTH] = MESSAGE_FOOTER;
	uint8_t message_size = sizeof(data);
	uint8_t data_size = message_size - HEADER_LENGTH - FOOTER_LENGTH;
	
  // Temporary testing, seeing if replacing now_recv_msg() works
	if (memcmp(data, expected_header, HEADER_LENGTH) != 0) {
    Serial.println("Header of message is incorrect");
    return;
  }
  if (memcmp(data + HEADER_LENGTH + data_size, expected_footer, FOOTER_LENGTH) != 0) {
    Serial.println("Footer of message is incorrect");
    return;
  }

  // Extract the data from the message
  memcpy(extracted_data, data + HEADER_LENGTH, data_size);
	Serial.println("Successfully extracted data from message");

  // Cast the data to be in the message struct format
  msg_content = reinterpret_cast<const message*>(extracted_data);

  // TODO: Have switch cases trigger interrupts to run tasks
  // Check message type
  switch (msg_content->msg_type) {
    case (message_type::Pairing):
      // // Check team ID to make sure it's your team
      // if (msg_content->team_id == team_id) {
      //   // Add new device to m_peers unordered map
      //   m_peers[recv_info->src_addr] = true;

      //   // Actually add to ESP NOW's peer list
      //   esp_now_peer_info_t * new_peer = malloc(sizeof(esp_now_peer_info_t));
      //   memset(new_peer, 0 sizeof(esp_now_peer_info_t));
      //   new_peer->channel = channel;
      //   new_peer->encrypt = is_encrypted;
      //   memcpy(new_peer->peer_addr, recv_info->src_addr, 6);


      //   if (esp_now_add_peer(new_peer) != ESP_OK) {
      //     Serial.println("Failed to add peer");
      //   }
      // }
      break;

    case (message_type::Location):
      // Modify marker on LED Strip corresponding to receiving device
      // TODO:
      break;
    case (message_type::SOS):
      // TODO:
      break;
    case (message_type::Alert):
      // TODO:
      break;
  }
}

uint8_t Nowcom::now_init() {
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

const uint8_t * Nowcom::get_self_mac() {
	return m_self_mac;
}

void Nowcom::set_encryption(bool is_encrypted) {
  this->is_encrypted = is_encrypted;
}

void Nowcom::set_channel(uint8_t channel) {
  this->channel = channel;
}

void Nowcom::set_team(uint8_t team_id) {
  this->team_id = team_id;
}

uint8_t Nowcom::now_send_msg(uint8_t * addr, uint8_t * data) {
	const uint8_t header[HEADER_LENGTH] = MESSAGE_HEADER;
	const uint8_t footer[FOOTER_LENGTH] = MESSAGE_FOOTER;
	const uint8_t data_size = sizeof(data);
	Serial.println(data_size);
	Serial.println(data_size + MESSAGE_OVERHEAD);
	uint8_t message[data_size + MESSAGE_OVERHEAD];
	
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

// TODO: put this in a task. Also do some sort of error checking for corrupted message
// uint8_t Nowcom::now_recv_msg(const uint8_t * message, uint8_t * data) {
// 	const uint8_t expected_header[HEADER_LENGTH] = MESSAGE_HEADER;
//   const uint8_t expected_footer[FOOTER_LENGTH] = MESSAGE_FOOTER;
// 	uint8_t message_size = sizeof(message);
// 	uint8_t data_size = message_size - HEADER_LENGTH - FOOTER_LENGTH;
	
// 	if (memcmp(message, expected_header, HEADER_LENGTH) != 0) {
//     return NOW_ERROR;
//   }
//   if (memcmp(message + HEADER_LENGTH + data_size, expected_footer, FOOTER_LENGTH) != 0) {
//     return NOW_ERROR;
//   }

//   memcpy(data, message + HEADER_LENGTH, data_size);
// 	return NOW_OK;
// }

void Nowcom::print_peer_mac_addresses() {
  Serial.printf("Peer MAC addresses: \n");
  // Go through each item in m_peers
  // Key: esp_now_peer_info_t, Val: 
  for (const auto& peer : m_peers) {
    Serial.printf("%2X:%2X:%2X:%2X:%2X:%2X\n", peer.first[0], peer.first[1], peer.first[2],
      peer.first[3], peer.first[4], peer.first[5]);
  }
}

uint8_t Nowcom::broadcast_init() {
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

uint8_t Nowcom::broadcast_mac_address() {
	return now_send_msg(m_broadcast.peer_addr, m_self_mac);
}
