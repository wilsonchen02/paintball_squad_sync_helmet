#include "GuidanceStrip.h"

#define LED_PIN   6

#define MAX_LED_COUNT   75

#define LED_COUNT   45
#define DEFAULT_BRIGHTNESS   5



GuidanceStrip gs(LED_COUNT, LED_PIN, DEFAULT_BRIGHTNESS);

void setup() {
  gs.begin();
  gs.clear();

  Serial.begin(115200);
}

//simulate input sequence
unsigned long lastInputTime = 0;
int inputStep = 0;
unsigned long inputIntervals[] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 5000, 5000, 5000};
uint8_t inputSequence[] = {1, 1, 1, 2, 0,1,1,1,2,2,2,2,1,0,0};
const int totalSteps = sizeof(inputSequence)/sizeof(inputSequence[0]);

void loop() {
  // Always update LEDs to keep blinking alive
  gs.update();

  // Check if it’s time to simulate the next input
  unsigned long now = millis();
  if (inputStep < totalSteps && now - lastInputTime >= inputIntervals[inputStep]) {
    gs.handleInput(inputSequence[inputStep]);
    lastInputTime = now;
    inputStep++;
  }

  delay(10);
}

//void loop() {
  // // Example: currentValue = 0b00 11 01 00  -> rrrr pppp bbbb rrrr
  // uint8_t value = 0b00110100;
  // gs.show_selector(4, value, 0);  // Blink section 0 (red)
  
  
  // // Example: currentValue = 0b0 1 1 0  -> rrrr bbbb bbbb rrrr
  // uint8_t value = 0b0110;
  // gs.show_selector(2, value, -1); // No blink

//}
