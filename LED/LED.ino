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


  //some simple testing for the guidance strip map mode
  gs.setLocation(0, 0, 0); //looking straight ahead from 0,0
  gs.setState(STATE_GUIDANCE);
  uint8_t mac1[6] = {1, 1, 1, 1, 1, 1};
  uint8_t mac2[6] = {2, 1, 1, 1, 1, 1};
  uint8_t mac3[6] = {3, 1, 1, 1, 1, 1};
  gs.addMate(mac1, 1.5, 5);
  gs.addMate(mac2, -1.5, 5);
  gs.addMate(mac3, 10, 7);


  gs.setLED(44, 0, 0, 255); //right limit
  gs.setLED(0, 0, 0, 255); //left limit
  gs.show();


  gs.showMap();
  delay(1000);
  gs.mateSOS(mac1);
  gs.showMap();

  delay(1000);
  gs.clearSOS();

  
  gs.showMap();
  delay(1000);
  gs.mateEngaged(mac1);

  gs.showMap();


  // gs.addMarker(-2, 2, -1);
  // gs.addObjective(0, 20);
  // gs.addMarker(50, 50 , -1);
  // gs.addObjective(10, -20); //wont show because it's behind you!
}

void loop() {
  //gs.update();
}

// //simulate input sequence
// unsigned long lastInputTime = 0;
// int inputStep = 0;
// unsigned long inputIntervals[] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 5000, 5000, 5000};
// uint8_t inputSequence[] = {1, 1, 1, 2, 0,1,1,1,2,2,2,2,1,0,0};
// const int totalSteps = sizeof(inputSequence)/sizeof(inputSequence[0]);

// void loop() {
//   // Always update LEDs to keep blinking alive
//   gs.update();

//   // Check if it’s time to simulate the next input
//   unsigned long now = millis();
//   if (inputStep < totalSteps && now - lastInputTime >= inputIntervals[inputStep]) {
//     gs.handlePhysicalInput(inputSequence[inputStep]);
//     lastInputTime = now;
//     inputStep++;
//   }

//   delay(10);
// }

//void loop() {
  // // Example: currentValue = 0b00 11 01 00  -> rrrr pppp bbbb rrrr
  // uint8_t value = 0b00110100;
  // gs.show_selector(4, value, 0);  // Blink section 0 (red)
  
  
  // // Example: currentValue = 0b0 1 1 0  -> rrrr bbbb bbbb rrrr
  // uint8_t value = 0b0110;
  // gs.show_selector(2, value, -1); // No blink

//}
