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
  if(batteryPercentage < BATTERY_WARNING_THRESHOLD && (index < BATTERY_WARNING_LEDS || index > numPixels-1-BATTERY_WARNING_LEDS)) return; //make room for battery warning
  
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

void GuidanceStrip::setBatteryPercentage(uint8_t b) {
  batteryPercentage = b;
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

  struct RGB {
    uint8_t r, g, b;
  };



  RGB palette[4];

  // Choose palette based on numColors
  if (numColors == 2) {
    // Game select: yellow and light blue
    palette[0] = {255, 255, 0};   // Yellow
    palette[1] = {0, 255, 255};   // Light Blue
    palette[2] = {0, 0, 0};       // unused
    palette[3] = {0, 0, 0};       // unused
  } else {
    // Team select: original 4 colors
    palette[0] = {255, 0, 0};     // Red
    palette[1] = {0, 0, 255};     // Blue
    palette[2] = {0, 255, 0};     // Green
    palette[3] = {255, 0, 255};   // Purple
  }

  // Draw each of the 4 sections
  uint16_t remainder = numPixels % 4;
  int start = 0;
  for (uint8_t section = 0; section < 4; section++) {
    uint8_t colorIndex = colors[section] % numColors;
    
    RGB color = palette[colorIndex];
    
    uint16_t extra = (section < remainder) ? 1 : 0;
    uint16_t end = start + sectionSize + extra;

    for (uint16_t i = start; i < end; i++) {
      if (section == currentPlace && blinkState) {
        setLED(i, 0, 0, 0); // Off during blink
      } else {
        setLED(i, color.r, color.g, color.b);
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

void GuidanceStrip::showBatteryLevel() {
  int totalBarLEDs = 10;                 
  int start = numPixels/2 - totalBarLEDs/2;
  
  int litLEDs = round((batteryPercentage / 100.0) * totalBarLEDs);
  if (batteryPercentage > 0 && litLEDs == 0) {   //Ensure 1 LED lights for 1–5%
    litLEDs = 1;
  }

  for (int i = 0; i < totalBarLEDs; i++) {
    if (i < litLEDs) {
      uint8_t r, g, b;

      if (batteryPercentage < 20) {
        r = 255; g = 0; b = 0;
      } else if (batteryPercentage < 30) {
        r = 255; g = 255; b = 0;
      } else {
        r = 0; g = 255; b = 0;
      }

      setLED(start + i, r, g, b);
    } else {
      // Unlit LEDs dim gray
      setLED(start + i, 50, 50, 50);
    }
  }

  show();
}

void GuidanceStrip::showBatteryWarning() {
  if (batteryPercentage < BATTERY_WARNING_THRESHOLD) {
    uint32_t color = strip.Color(255, 0, 0);

    for(int i = 0; i < BATTERY_WARNING_LEDS; i++) {
      strip.setPixelColor(i, color);
      strip.setPixelColor(numPixels - 1 - i, color);
    }
  }

  strip.show();
}




void GuidanceStrip::showGPSConnecting() {
    static uint8_t gpsLitLEDs = 0;
    static unsigned long gpsLastUpdate = 0;
    const unsigned long GPS_STEP_INTERVAL = 1000; //ms

    int totalBarLEDs = 5;
    int start = numPixels/2 - totalBarLEDs/2;
    unsigned long now = millis();

    if (now - gpsLastUpdate >= GPS_STEP_INTERVAL) {
      gpsLastUpdate = now;
      gpsLitLEDs++;

      if (gpsLitLEDs > totalBarLEDs)
        gpsLitLEDs = 0; 
    }

    for (int i = 0; i < totalBarLEDs; i++) {
      setLED(start + i, 150, 150, 150);
    }

    for (int i = 0; i < gpsLitLEDs; i++) {
      setLED(start + i, 255, 255, 0);
    }

    show();
}


void GuidanceStrip::showGPSConnected() {
    int totalBarLEDs = 5;
    int start = numPixels/2 - totalBarLEDs/2;

    for (int i = 0; i < totalBarLEDs; i++) {
      setLED(start + i, 0, 255, 0);
    }

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


void GuidanceStrip::addMarker(float x, float y, int32_t ttl) { // long, lat, time to live
  uint8_t zeroMac[6] = {0, 0, 0, 0, 0, 0};
  mapElems.emplace_back(zeroMac, MARKER_ELEM, x, y, ttl);
}

void GuidanceStrip::addObjective(float x, float y) {  // long, lat
    uint8_t zeroMac[6] = {0, 0, 0, 0, 0, 0};
    mapElems.emplace_back(zeroMac, OBJECTIVE_ELEM, x, y, -1);
}

// Removes all the map objectives in the mapElems vector
void GuidanceStrip::removeObjectives() {
  for (auto it = mapElems.begin(); it != mapElems.end();) {
    if (it->type == OBJECTIVE_ELEM) {
      it = mapElems.erase(it);
      Serial.println("erased 1");
    }
    else {
      it++;
    }
  } 
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

void GuidanceStrip::mateSOS() {
  inSOS = true;
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

bool GuidanceStrip::isInSOS() {
  return inSOS;
}

uint8_t GuidanceStrip::applyLinearBrightness(uint8_t v, float brightness) {
    // sRGB to linear (approx: c^2.2)
    float c = (v / 255.0f);
    float lin = powf(c, 2.2f);

    // apply brightness in linear space
    lin *= brightness;

    // linear to sRGB
    float out = powf(lin, 1.0f / 2.2f);

    // clamp and convert back to 0–255
    if (out < 0.0f) out = 0.0f;
    if (out > 1.0f) out = 1.0f;

    return (uint8_t)(out * 255.0f + 0.5f);
}



void GuidanceStrip::showMap() {
  // --- Tuning Parameters for Brightness ---
  const float minDistance = MIN_DISTANCE;   // Objects closer than this are at full brightness
  const float maxDistance = MAX_DISTANCE;  // Objects farther than this are shown at minimum brightness
  const float minBrightnessFactor =  getMinBrightnessFactor(brightness);  // Minimum brightness for objects at maxDistance as a percentage
  
  std::vector<float> ledBrightness(numPixels, 0.0f); 
  std::vector<Color> ledColor(numPixels, {0, 0, 0}); 
  std::vector<int> ledPriority(numPixels, 0);
  
  for(auto &elem : mapElems) {
    float dx = elem.x - myX;
    float dy = elem.y - myY;

    // // Calculate the distance to the element
    // // (Pythagorean theorem: a^2 + b^2 = c^2)
    // float distance = sqrt(dx * dx + dy * dy);

    // --- Calculate geodesic distance using the Haversine formula ---
    const float R = 6371000.0f; // Earth radius in meters

    float lat1 = radians(myY);
    float lon1 = radians(myX);
    float lat2 = radians(elem.y);
    float lon2 = radians(elem.x);

    float dLat = lat2 - lat1;
    float dLon = lon2 - lon1;

    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(lat1) * cos(lat2) *
              sin(dLon / 2) * sin(dLon / 2);

    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    float distance = R * c; // Distance in meters

    // Calculate the angle
    float targetBearingDeg = atan2(dx, dy) * 180.0 / PI;
    float relativeAngle = targetBearingDeg - myHeading;

    while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
    while (relativeAngle < -180.0f) relativeAngle += 360.0f;
    
    //Light border if behind in SOS
    if(inSOS && std::abs(relativeAngle) > 90) {
      Color c = COLOR_SOS;
      if(elem.r != c.r || elem.g != c.g || elem.b != c.b)
        continue;
      if(relativeAngle > -90) {
        for(int i = 0; i < SOS_BEHIND_LEDS; i++) {
          setLED(numPixels-1-i, c.r, c.g, c.b);
        }
      }
      if(relativeAngle < 90) {
        for(int i = 0; i < SOS_BEHIND_LEDS; i++) {
          setLED(i, c.r, c.g, c.b);
        }
      }
      show();
      continue;
    }

    // Ignore if behind
    if (std::abs(relativeAngle) > 90) { 
      continue;
    }

    // Map distance to a brightness factor (0.0-1.0)
    float brightnessFactor;
    if (distance < minDistance) {
      brightnessFactor = 1.0; // At max brightness if closer than minDistance
    } 
    else if (distance > maxDistance) {
      brightnessFactor = 0;
    }
    else {
      // As distance increases, brightnessFactor decreases from 1.0 to 0.0
      // brightnessFactor = (maxDistance - distance) / (maxDistance - minDistance); //linear
      brightnessFactor = exp(-6 * (distance - minDistance) / (maxDistance - minDistance)); //exponential decay
    }
    
    //ensure lights stay on regardless of brightness
    //(is a value between minBrightnessFactor and 1)
    brightnessFactor = minBrightnessFactor + (brightnessFactor * (1.0 - minBrightnessFactor));
    

    if (distance > 9999) continue; //hard limit


    uint8_t r = applyLinearBrightness(elem.r, brightnessFactor);
    uint8_t g = applyLinearBrightness(elem.g, brightnessFactor);
    uint8_t b = applyLinearBrightness(elem.b, brightnessFactor);


    int elemPriority = elem.type == MATE_ELEM ? 1 : 0;

    float fraction = (relativeAngle + 90.0) / 180.0;
    int ledIndex = round(fraction * (numPixels - 1));

    //only set LED if priority is higher or if priority is equal and brightness is higher
    if (elemPriority > ledPriority[ledIndex] ||
       (elemPriority == ledPriority[ledIndex] && brightnessFactor > ledBrightness[ledIndex])) {
        ledPriority[ledIndex] = elemPriority;
        ledBrightness[ledIndex] = brightnessFactor;
        ledColor[ledIndex] = {r, g, b};
    }


    // Serial.println(elem.type);
    //  Serial.println(brightnessFactor);
    //  Serial.println(distance);
    //  Serial.println(minBrightnessFactor);
    // Serial.println(brightness);
    //  Serial.print(r); Serial.print(", "); Serial.print(g); Serial.print(", "); Serial.println(b);
    //  Serial.println("----------");

  }
  
  // set all LED colors
  for (int i = 0; i < numPixels; i++) {
    setLED(i, ledColor[i].r, ledColor[i].g, ledColor[i].b);
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
      switch (state) {
        case STATE_GAME_SELECT: {
          setState(STATE_TEAM_SELECT);
          // // Clear previous objectives in mapElems
           removeObjectives();
          // Set new objectives
          int current_code = getGameCode();
          Serial.printf("Game Code: %d\n", current_code);

          // Hard coded objectives depending on game selected
          if(current_code == 2) { // Map 1
            Serial.println("Map 1 Selected");
            addObjective(OBJ_WEST_TOWER_LONG, OBJ_WEST_TOWER_LAT);
          }
          else if(current_code == 6) {  // Map 2
            Serial.println("Map 2 Selected");
            addObjective(OBJ_TWO_HEARTS_LONG, OBJ_TWO_HEARTS_LAT);
          }
          break;
        }
        case STATE_TEAM_SELECT: {
          setState(STATE_GUIDANCE);
          break;
        }
        case STATE_GPS_CONNECTING: {
          setState(STATE_GAME_SELECT);
          break;
        }
        case STATE_GUIDANCE: {
          setState(STATE_GAME_SELECT);
          break;
        }
      }
      break;

    case 1: // Button 1 
      switch (state) {
        case STATE_GAME_SELECT: {
          if(++gameSelectorPos > 3) gameSelectorPos = 0;
          //update();
          break;
        }
        case STATE_TEAM_SELECT: {
          if(++teamSelectorPos > 3) teamSelectorPos = 0;
          //update();
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
            gameSelectorColors[3] *     8     +
            gameSelectorColors[2] *     4     +
            gameSelectorColors[1] *     2     +
            gameSelectorColors[0]
            + 1;
          //update();
          break;
        }
        case STATE_TEAM_SELECT: {
          teamSelectorColors[teamSelectorPos] = (teamSelectorColors[teamSelectorPos] + 1) % 4;
          teamCode =
            teamSelectorColors[3] *     64    +
            teamSelectorColors[2] *     16    +
            teamSelectorColors[1] *     4     +
            teamSelectorColors[0];
          //update();
          break;
        }
        case STATE_GUIDANCE: {
          //signal that I am engaged
          //on the outside, sends a message that will result in mateEngaged(mac) being called for others
          break;
        }
      }
      break;

      case 3: // Brightness button pressed
        setBrightness(getNextBrightness(brightness));
        break;

      case 4: // Button 1 held down
        switch (state) {
          case STATE_GUIDANCE: {
            if(!inSOS) {
            //on the outside, sends a message that will result in mateSOS(mac) being called for others
            }
            else {
              //on the outside, sends a message that will result in clearSOS() being called for others
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
  if(batteryPercentage < 1) return;

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

    case STATE_GUIDANCE: {
      if(myX == 444.0) { //GPS not connected
        setState(STATE_GPS_CONNECTING);
      }

      showMap();
      break;
    }
    case STATE_GPS_CONNECTING: {
      if(myX != 444.0) { //GPS connected
        showGPSConnected();
        delay(2000);
        setState(STATE_GUIDANCE);
        break;
      }

      showGPSConnecting();
      break;
    }

    default: {
      // Unknown state, fallback
      clear();
      break;
    }
  }

  if(batteryPercentage < 20) {
    showBatteryWarning();
  }
}

