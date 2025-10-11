#include "GuidanceStrip.h"

GuidanceStrip::GuidanceStrip(uint16_t numPixels, uint8_t pin, uint8_t brightnessLevel)
  : strip(numPixels, pin, NEO_GRB + NEO_KHZ800), numPixels(numPixels) {
  setBrightness(brightnessLevel);
}

void GuidanceStrip::begin() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void GuidanceStrip::setLED(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
  if (index < numPixels) {
    strip.setPixelColor(index, strip.Color(r, g, b));
  }
}

void GuidanceStrip::setBrightness(uint8_t brightnessLevel) {
  brightness = brightnessLevel;
  strip.setBrightness(brightnessLevel);
}

void GuidanceStrip::show() {
  strip.show();
}

void GuidanceStrip::clear() {
  strip.clear();
}

uint8_t GuidanceStrip::getState() {
  return state;
}

void GuidanceStrip::setState(uint8_t newState) {
  state = newState;
  update();
}



// -------------------------------------------
// Show selector visualization
// numColors = number of available colors (2–4)
// colors[] = array of 4 color indices for each section
// currentPlace = 0–3 (blinking section) or -1 for no blink
// -------------------------------------------
void GuidanceStrip::showSelector(uint8_t numColors, uint8_t colors[4], int8_t currentPlace) {
  uint16_t sectionSize = numPixels / 4;
  static bool blinkState = false;
  static unsigned long lastBlinkTime = 0;
  unsigned long now = millis();

  // Blink toggle
  if (now - lastBlinkTime > BLINK_SPEED) {
    blinkState = !blinkState;
    lastBlinkTime = now;
  }

  // Choose palette based on numColors
  uint32_t palette[4];
  if (numColors == 2) {
    // Game select: yellow and light blue
    palette[0] = strip.Color(255, 255, 0);   // Yellow
    palette[1] = strip.Color(0, 255, 255);   // Light Blue
    palette[2] = palette[3] = 0;             // unused
  } else {
    // Team select: original 4 colors
    palette[0] = strip.Color(255, 0, 0);     // Red
    palette[1] = strip.Color(0, 0, 255);     // Blue
    palette[2] = strip.Color(0, 255, 0);     // Green
    palette[3] = strip.Color(255, 0, 255);   // Purple
  }

  // Draw each of the 4 sections
  for (uint8_t section = 0; section < 4; section++) {
    uint8_t colorIndex = colors[section] % numColors;
    uint32_t color = palette[colorIndex];

    uint16_t start = section * sectionSize;
    uint16_t end = (section == 3) ? numPixels : start + sectionSize;

    for (uint16_t i = start; i < end; i++) {
      if (section == currentPlace && blinkState) {
        strip.setPixelColor(i, 0, 0, 0); // Off during blink
      } else {
        strip.setPixelColor(i, color);
      }
    }
  }

  strip.show();
}


void GuidanceStrip::showBrightnessPreview() {
  //TODO: come up with better scaling for dim/far vs bright/close markers
  setLED(numPixels/8, 255/8, 255/8, 255/8);
  setLED(2*numPixels/8, 2*255/8, 2*255/8, 2*255/8);
  setLED(3*numPixels/8, 3*255/8, 3*255/8, 3*255/8);
  setLED(4*numPixels/8, 4*255/8, 4*255/8, 4*255/8);
  setLED(5*numPixels/8, 5*255/8, 5*255/8, 5*255/8);
  setLED(6*numPixels/8, 6*255/8, 6*255/8, 6*255/8);
  setLED(7*numPixels/8, 7*255/8, 7*255/8, 7*255/8);
  setLED(numPixels-1, 255, 255, 255);

  show();
}


void GuidanceStrip::handleInput(uint8_t input) {
  // 0 = mode button
  // 1 = action button 1
  // 2 = action button 2

  switch (input) {
    case 0: // Mode button cycles states
      setState((getState() + 1) % 4);
      break;

    case 1: // Button 1 
      switch (state) {
        case STATE_GAME_SELECT: {
          if(gameSelectorPos++ > 3) gameSelectorPos = 0;
          update();
          break;
        }
        case STATE_TEAM_SELECT: {
          if(teamSelectorPos++ > 3) teamSelectorPos = 0;
          update();
          break;
        }
        case STATE_BRIGHTNESS: {
          //TODO: determine reasonable brightness threshold
          brightness += 2;

          break;
        }
        case STATE_GUIDANCE: {
          //TODO: signal that I am out of the game
          break;
        }
      }
      break;

    case 2: // Button 2
      switch (state) {
        case STATE_GAME_SELECT: {
          gameSelectorColors[gameSelectorPos] = (gameSelectorColors[gameSelectorPos] + 1) % 2;
          update();
          break;
        }
        case STATE_TEAM_SELECT: {
          teamSelectorColors[teamSelectorPos] = (teamSelectorColors[teamSelectorPos] + 1) % 4;
          update();
          break;
        }
        case STATE_BRIGHTNESS: {
          //TODO: determine reasonable brightness threshold
          brightness -= 2;
          break;
        }
        case STATE_GUIDANCE: {
          //TODO: signal that I am out of the game
          break;
        }
      }
      break;

    default:
      // No action
      break;
  }
}


void GuidanceStrip::update() {
  clear();
  switch (state) {
    case STATE_GAME_SELECT: {
      showSelector(2, gameSelectorColors, gameSelectorPos);
      break;
    }

    case STATE_TEAM_SELECT: {
      // Convert the 4 section color indices into a single "value" like base-4
      // e.g. colors = {0,1,3,0} → value = 0b00 11 01 00 (base-4)
      showSelector(4, teamSelectorColors, teamSelectorPos);
      break;
    }

    case STATE_BRIGHTNESS: {
      setBrightness(brightness);
      showBrightnessPreview();
      break;
    }

    case STATE_GUIDANCE: {
      // TODO: guidance mode
      break;
    }

    default: {
      // Unknown state, fallback
      clear();
      break;
    }
  }
}

