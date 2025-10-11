#ifndef GUIDANCE_STRIP_H
#define GUIDANCE_STRIP_H

#include <Adafruit_NeoPixel.h>

#define STATE_GAME_SELECT 0
#define STATE_TEAM_SELECT 1 
#define STATE_BRIGHTNESS 2 
#define STATE_GUIDANCE 3

#define BLINK_SPEED 100 //ms


class GuidanceStrip {
public:
  GuidanceStrip(uint16_t numPixels, uint8_t pin, uint8_t brightness = 50);

  void begin();
  void setLED(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t brightnessLevel);
  void show();
  void clear();

  void showSelector(uint8_t numColors, uint8_t colors[4], int8_t currentPlace);
  void showBrightnessPreview();

  uint8_t getState();
  void setState(uint8_t newState);
  void handleInput(uint8_t input);
  void update();


private:
  Adafruit_NeoPixel strip;
  uint16_t numPixels;
  uint8_t state = 0;
  uint8_t gameCode;
  uint8_t gameSelectorPos;
  uint8_t gameSelectorColors[4] = {0, 0, 0, 0};
  uint8_t teamCode;
  uint8_t teamSelectorPos;
  uint8_t teamSelectorColors[4] = {0, 0, 0, 0};
  uint8_t brightness;


};

#endif
