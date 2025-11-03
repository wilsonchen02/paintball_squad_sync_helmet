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

uint8_t GuidanceStrip::getGameCode() {
  return gameCode;
}

uint8_t GuidanceStrip::getTeamCode() {
  return teamCode;
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
  uint16_t remainder = numPixels % 4;
  int start = 0;
  for (uint8_t section = 0; section < 4; section++) {
    uint8_t colorIndex = colors[section] % numColors;
    uint32_t color = palette[colorIndex];

    uint16_t extra = (section < remainder) ? 1 : 0;
    uint16_t end = start + sectionSize + extra;

    for (uint16_t i = start; i < end; i++) {
      if (section == currentPlace && blinkState) {
        strip.setPixelColor(i, 0, 0, 0); // Off during blink
      } else {
        strip.setPixelColor(i, color);
      }
    }
    start = end; // next section starts where this one ended
  }

  show();
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



void GuidanceStrip::addMate(uint8_t mac[6], float x, float y) {
  // Check if element with same MAC already exists
  for (auto &elem : mapElems) {
    if (elem.matchesMac(mac)) {
      //Update position
      elem.x = x;
      elem.y = y;
      elem.ttl = MATE_TTL;
      elem.lastUpdate = millis();
      return;      
    }
  }

  // If not found, create a new one
  mapElems.emplace_back(mac, MATE_ELEM, x, y, 5000);
}

void GuidanceStrip::mateEngaged(uint8_t mac[6]) {
  for (auto &elem : mapElems) {
    if (elem.matchesMac(mac)) {
      Color c = COLOR_ENGAGED;
      elem.r = c.r;
      elem.g = c.g;
      elem.b = c.b;
      elem.engagedUntil = millis() + TIME_ENGAGED;
      return;
    }
  }
}

void GuidanceStrip::mateEliminated(uint8_t mac[6]) {
  for (auto &elem : mapElems) {
    if (elem.matchesMac(mac)) {
      Color c = COLOR_ELIMINATED;
      elem.r = c.r;
      elem.g = c.g;
      elem.b = c.b;
    }
  }
}


void GuidanceStrip::addMarker(float x, float y, int32_t ttl) {
  uint8_t zeroMac[6] = {0, 0, 0, 0, 0, 0};
  mapElems.emplace_back(zeroMac, MARKER_ELEM, x, y, ttl);
}

void GuidanceStrip::addObjective(float x, float y) {
    uint8_t zeroMac[6] = {0, 0, 0, 0, 0, 0};
    mapElems.emplace_back(zeroMac, OBJECTIVE_ELEM, x, y, -1);
}

void GuidanceStrip::mateSOS(uint8_t mac[6]) {
  inSOS = true;
  for (auto &elem : mapElems) {
    if (elem.matchesMac(mac)) {
      Color c = COLOR_SOS;
      elem.r = c.r;
      elem.g = c.g;
      elem.b = c.b;
    }
    else {
      //hide all other elems
      elem.r = 0;
      elem.g = 0;
      elem.b = 0;
    }
  }
}

void GuidanceStrip::clearSOS() {
  inSOS = false;
  for (auto &elem : mapElems) {
    if (elem.type == MATE_ELEM) {
      // update color
      Color c = COLOR_MATE;
      elem.r = c.r;
      elem.g = c.g;
      elem.b = c.b;
    }
    if (elem.type == OBJECTIVE_ELEM) {
      // update color
      Color c = COLOR_OBJECTIVE;
      elem.r = c.r;
      elem.g = c.g;
      elem.b = c.b;
    }
    if (elem.type == MARKER_ELEM) {
      // update color
      Color c = COLOR_MARKER;
      elem.r = c.r;
      elem.g = c.g;
      elem.b = c.b;
    }
  }
}

void GuidanceStrip::showMap() {
  // --- Tuning Parameters for Brightness ---
  const float minDistance = MIN_DISTANCE;   // Objects closer than this are at full brightness
  const float maxDistance = MAX_DISTANCE;  // Objects farther than this are not shown.
  const float minBrightnessFactor = 0.01;   // Minimum brightness for objects at maxDistance as a percentage
  
  //TODO: set priorities for which mapelement types overtake others
  for(auto &elem : mapElems) {
    float dx = elem.x - myX;
    float dy = elem.y - myY;

    // Calculate the distance to the element
    // (Pythagorean theorem: a^2 + b^2 = c^2)
    float distance = sqrt(dx * dx + dy * dy);

    // Calculate the angle
    float targetBearingDeg = atan2(dx, dy) * 180.0 / PI;
    float relativeAngle = targetBearingDeg - myHeading;

    while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
    while (relativeAngle < -180.0f) relativeAngle += 360.0f;
    
    // Ignore if behind too far
    if (std::abs(relativeAngle) > 90 || distance > maxDistance) { 
      continue;
    }

    // Map distance to a brightness factor (0.0-1.0)
    float brightnessFactor;
    if (distance < minDistance) {
      brightnessFactor = 1.0; // At max brightness if closer than minDistance
    } else {
      // As distance increases, brightnessFactor decreases from 1.0 to 0.0
      brightnessFactor = (maxDistance - distance) / (maxDistance - minDistance);
    }
    
    // Scale so it doesn't go totally out until hitting minimum
    brightnessFactor = minBrightnessFactor + (brightnessFactor * (1.0 - minBrightnessFactor));

    uint8_t r = elem.r * brightnessFactor;
    uint8_t g = elem.g * brightnessFactor;
    uint8_t b = elem.b * brightnessFactor;

    float fraction = (relativeAngle + 90.0) / 180.0;
    int ledIndex = round(fraction * (numPixels - 1));

    setLED(ledIndex, r, g, b);
  }
  show();
}

void GuidanceStrip::handleMsgInput(uint8_t input) {
  //TODO: handle new incoming message
}

void GuidanceStrip::setLocation(float x, float y, float heading) {
  myX = x;
  myY = y;
  myHeading = heading;
};


void GuidanceStrip::handlePhysicalInput(uint8_t input) {
  // 0 = mode button
  // 1 = action button 1
  // 2 = action button 2

  switch (input) {
    case 0: // Mode button cycles states
      setState((getState() + 1) % 3);
      break;

    case 1: // Button 1 
      switch (state) {
        case STATE_GAME_SELECT: {
          if(++gameSelectorPos > 3) gameSelectorPos = 0;
          update();
          break;
        }
        case STATE_TEAM_SELECT: {
          if(++teamSelectorPos > 3) teamSelectorPos = 0;
          update();
          break;
        }
        case STATE_BRIGHTNESS: { //UNUSED
          //TODO: determine reasonable brightness threshold
          brightness += 2;

          break;
        }
        case STATE_GUIDANCE: {
          //signal that I am out of the game
          //TODO: sends a message that will result in mateEliminated(mac) being called for others
          setState(STATE_GAME_SELECT); //returns to game select screen, ending msg sends and eventually ttl goes to 0
          break;
        }
      }
      break;

    case 2: // Button 2
      switch (state) {
        case STATE_GAME_SELECT: {
          gameSelectorColors[gameSelectorPos] = (gameSelectorColors[gameSelectorPos] + 1) % 2;
          gameCode =
            gameSelectorColors[3] * pow(2, 3) +
            gameSelectorColors[2] * pow(2, 2) +
            gameSelectorColors[1] *     2     +
            gameSelectorColors[0];
          update();
          break;
        }
        case STATE_TEAM_SELECT: {
          teamSelectorColors[teamSelectorPos] = (teamSelectorColors[teamSelectorPos] + 1) % 4;
          teamCode =
            teamSelectorColors[3] * pow(4, 3) +
            teamSelectorColors[2] * pow(4, 2) +
            teamSelectorColors[1] *     4     +
            teamSelectorColors[0];
          update();
          break;
        }
        case STATE_BRIGHTNESS: { //UNUSED
          //TODO: determine reasonable brightness threshold
          brightness -= 2;
          break;
        }
        case STATE_GUIDANCE: {
          //signal that I am engaged
          //TODO: sends a message that will result in mateEngaged(mac) being called for others
          break;
        }
      }
      break;

      case 3: // Brightness button pressed
        brightness+=2;
        if(brightness > 10) brightness = 1;
        setBrightness(brightness);
        break;

      case 4: // Button 1 held down
        switch (state) {
          case STATE_GUIDANCE: {
            if(!inSOS) {
            //TODO: sends a message that will result in mateSOS(mac) being called for others
            }
            else {
              //TODO: sends a message that will result in clearSOS() being called for others
            }
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

  // time to live - remove expired elements
  uint32_t now = millis();


  // remove expired elements
  for (auto it = mapElems.begin(); it != mapElems.end(); ) {
    if (it->expired(now)) {
      it = mapElems.erase(it);
    } 
    else {
      // check if engagement should be reverted
      if (it->engagedUntil != 0 && now >= it->engagedUntil) {
        it->engagedUntil = 0;
        if (it->type == MATE_ELEM) {
          Color c = COLOR_MATE;
          it->r = c.r;
          it->g = c.g;
          it->b = c.b;
        }
      }
      ++it;
    }
  }




  switch (state) {
    case STATE_GAME_SELECT: {
      showSelector(2, gameSelectorColors, gameSelectorPos);
      break;
    }

    case STATE_TEAM_SELECT: {
      // Convert the 4 section color indices into a single value like base-4
      // e.g. colors = {0,1,3,0} → value = 0b00 11 01 00 (base-4)
      showSelector(4, teamSelectorColors, teamSelectorPos);
      break;
    }

    case STATE_BRIGHTNESS: { //UNUSED
      setBrightness(brightness);
      showBrightnessPreview();
      break;
    }

    case STATE_GUIDANCE: {
      showMap();
      break;
    }

    default: {
      // Unknown state, fallback
      clear();
      break;
    }
  }
}

