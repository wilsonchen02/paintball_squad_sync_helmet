#ifndef GUIDANCE_STRIP_H
#define GUIDANCE_STRIP_H

#include <Adafruit_NeoPixel.h>

#define STATE_GAME_SELECT 0
#define STATE_TEAM_SELECT 1 
#define STATE_GUIDANCE 2

#define STATE_BRIGHTNESS 3 //UNUSED NOW 


#define BLINK_SPEED 100 //ms

#define MATE_ELEM 0
#define OBJECTIVE_ELEM 1
#define MARKER_ELEM 2

#define MATE_TTL 3000 //refresh TTL to be change

//COLORS
typedef struct {
    int r;
    int g;
    int b;
} Color;

#define COLOR_MATE       ((Color){0,   255, 0})
#define COLOR_SOS        ((Color){0,   0,   255})
#define COLOR_ENGAGED    ((Color){255, 55,  0})
#define COLOR_OBJECTIVE  ((Color){255, 255, 0})
#define COLOR_MARKER     ((Color){255, 0,   0})
#define COLOR_ELIMINATED ((Color){255, 255, 255})
#define COLOR_DEFAULT    ((Color){255, 0, 255})

#define TIME_ENGAGED 10000 //ms

#define MAX_DISTANCE 100
#define MIN_DISTANCE 0.01





struct MapElement {
  uint8_t macAddr[6];
  uint8_t type;
  float x, y;
  uint8_t r, g, b;
  uint32_t ttl;
  uint32_t lastUpdate;
  int32_t engagedUntil;

  // Constructor
MapElement(const uint8_t mac[6], uint8_t t, float x, float y, int32_t ttl)
  : type(t), x(x), y(y), ttl(ttl), lastUpdate(millis()), engagedUntil(0)
{
    memcpy(macAddr, mac, 6);

    Color color;

    switch (type) {
      case MATE_ELEM:       color = COLOR_MATE; break;
      case MARKER_ELEM:     color = COLOR_MARKER; break;
      case OBJECTIVE_ELEM:  color = COLOR_OBJECTIVE; break;
      default:              color = COLOR_DEFAULT; break;
    }

    r = color.r;
    g = color.g;
    b = color.b;
}

  bool matchesMac(const uint8_t otherMac[6]) const {
    return memcmp(macAddr, otherMac, 6) == 0;
  }
  
  bool expired(uint32_t now) const {
    if (ttl < 0) return false;            // -1 = infinite
    return (now - lastUpdate) >= ttl;
  }
};




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
  uint8_t getGameCode();
  uint8_t getTeamCode();
  void setState(uint8_t newState);
  void handlePhysicalInput(uint8_t input);
  void update();

  void addMate(uint8_t mac[6], float x, float y);
  void addMarker(float x, float y, int32_t ttl);
  void addObjective(float x, float y);

  void mateEngaged(uint8_t mac[6]);
  void mateEliminated(uint8_t mac[6]);
  void mateSOS(uint8_t mac[6]);
  void mateSOS();
  void clearSOS();
  bool isInSOS();

  void showMap();

//        +Y (dy > 0)
//         ↑
//         |
// ← -X    • (0,0)    +X →
//         |
//         ↓
//       -Y (dy < 0)


  

  void handleMsgInput(uint8_t input);
  void setLocation(float x, float y, float heading);


  //nowcom message format:
    // struct message {
    //   uint8_t team_id;
    //   message_type msg_type;
    //   float x;
    //   float y;
    // };


private:
  Adafruit_NeoPixel strip;
  uint16_t numPixels;
  uint8_t state = 0;
  uint8_t gameCode = 1;
  uint8_t gameSelectorPos;
  uint8_t gameSelectorColors[4] = {0, 0, 0, 0};
  uint8_t teamCode;
  uint8_t teamSelectorPos;
  uint8_t teamSelectorColors[4] = {0, 0, 0, 0};
  uint8_t brightness;
  bool inSOS = false;

  std::vector<MapElement> mapElems;

  float myX; //latitude
  float myY; //longitude
  float myHeading; //-180 to 180

};

#endif
