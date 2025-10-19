#ifndef TEST_H
#define TEST_H

#include <Arduino.h>
#include "espnow.h"

// ESP-NOW MS1 Demo
void demo_ms1_setup();
void demo_ms1_task(void *pvParameters);
void demo_ms1_espnow_print(uint8_t data_type, const uint8_t * data, uint8_t payload_len);

#endif
