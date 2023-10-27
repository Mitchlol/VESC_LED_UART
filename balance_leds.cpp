#define FASTLED_INTERNAL
#include "./src/Adafruit_NeoPixel/Adafruit_NeoPixel.h"
#include "config.cpp"
#include "voltagetable.cpp"

// LED Type
#define LED_TYPE NEO_GRBW

// LED Wiring
// Note: Forward in this instance means postive ERPM, which can actually be backwards if your motor direction is reversed.
#define LED_PIN_FORWARD 10
#define LED_PIN_BACKWARD 11
#define NUM_LEDS_FORWARD 10
#define NUM_LEDS_BACKWARD 10
#define STARTUP_FORWARD true

// Colors and brightnesses (User selectable between 4 idle/active brightness levels, and 7 colors, these can be set to anything)
#define COLORS (unsigned long[]){0xFFFFFFFF, 0x000000FF, 0x0000FFFF, 0x0000FF00, 0x00FFFF00, 0x00FF0000, 0x00FF00FF}
#define BRIGHTNESS (int[]){0xFF, 0x88, 0x44, 0x00}
#define IDLE_BRIGHTNESS (int[]){0x0A, 0x05, 0x01, 0x00}

// Timings
#define IDLE_DELAY 2000
#define FADE_OUT_BY 10
#define FADE_IN_BY 30
#define FADE_DIRECTION_SPEED 20
#define FADE_BATTERY_SPEED 20

class BalanceLEDs {
  private:
    Config& config;
    Adafruit_NeoPixel forwardPixels{NUM_LEDS_FORWARD, LED_PIN_FORWARD, LED_TYPE + NEO_KHZ800};
    Adafruit_NeoPixel backwardPixels{NUM_LEDS_BACKWARD, LED_PIN_BACKWARD, LED_TYPE + NEO_KHZ800};
    VoltageTable voltageTable;

    // Runtime vars
    bool directionIsForward;
    uint8_t currentBrightness;
    uint8_t targetBrightness;
    uint8_t previousBrightnessState;
    uint8_t previousForwardColorState;
    uint8_t previousBackwardColorState;
    long idleTimer;
    bool isIdle;
    bool isIdleAndFaded;
    uint32_t previousColorForward;
    uint32_t previousColorBackward;

    void fadeForwardColor(uint32_t desiredColor, uint8_t fadeAmount){
      uint8_t w = desiredColor >> 24;
      uint8_t r = desiredColor >> 16;
      uint8_t g = desiredColor >> 8;
      uint8_t b = desiredColor;
      uint8_t white = previousColorForward >> 24;
      uint8_t red = previousColorForward >> 16;
      uint8_t green = previousColorForward >> 8;
      uint8_t blue = previousColorForward;
      uint32_t color;
      if(abs(w - white) < fadeAmount){
        color |= (uint32_t)w << 24;
      }else if(w > red){
        color |= (uint32_t)(white + fadeAmount) << 24;
      }else{
        color |= (uint32_t)(white - fadeAmount) << 24;
      }
      if(abs(r - red) < fadeAmount){
        color |= (uint32_t)r << 16;
      }else if(r > red){
        color |= (uint32_t)(red + fadeAmount) << 16;
      }else{
        color |= (uint32_t)(red - fadeAmount) << 16;
      }
      if(abs(g - green) < fadeAmount){
        color |= (uint32_t)g << 8;
      }else if(g > green){
        color |= (uint32_t)(green + fadeAmount) << 8;
      }else{
        color |= (uint32_t)(green - fadeAmount) << 8;
      }
      if(abs(b - blue) < fadeAmount){
        color |= b;
      }else if(b > blue){
        color |= (blue + fadeAmount);
      }else{
        color |= (blue - fadeAmount);
      }
      forwardPixels.fill(color);
      previousColorForward = color;
    }

    void fadeBackwardColor(uint32_t desiredColor, uint8_t fadeAmount){
      uint8_t w = desiredColor >> 24;
      uint8_t r = desiredColor >> 16;
      uint8_t g = desiredColor >> 8;
      uint8_t b = desiredColor;
      uint8_t white = previousColorBackward >> 24;
      uint8_t red = previousColorBackward >> 16;
      uint8_t green = previousColorBackward >> 8;
      uint8_t blue = previousColorBackward;
      uint32_t color;
      if(abs(w - white) < fadeAmount){
        color |= (uint32_t)w << 24;
      }else if(w > red){
        color |= (uint32_t)(white + fadeAmount) << 24;
      }else{
        color |= (uint32_t)(white - fadeAmount) << 24;
      }
      if(abs(r - red) < fadeAmount){
        color |= (uint32_t)r << 16;
      }else if(r > red){
        color |= (uint32_t)(red + fadeAmount) << 16;
      }else{
        color |= (uint32_t)(red - fadeAmount) << 16;
      }
      if(abs(g - green) < fadeAmount){
        color |= (uint32_t)g << 8;
      }else if(g > green){
        color |= (uint32_t)(green + fadeAmount) << 8;
      }else{
        color |= (uint32_t)(green - fadeAmount) << 8;
      }
      if(abs(b - blue) < fadeAmount){
        color |= b;
      }else if(b > blue){
        color |= (blue + fadeAmount);
      }else{
        color |= (blue - fadeAmount);
      }
      backwardPixels.fill(color);
      previousColorBackward = color;
    }

  public:
    BalanceLEDs(Config& mConfig): config(mConfig) {}  
    
    void setup(){

      // Init LEDs
      forwardPixels.begin();
      backwardPixels.begin();
      forwardPixels.setBrightness(0);
      backwardPixels.setBrightness(0);
      previousBrightnessState = config.brightnessState;
      previousForwardColorState = config.forwardColorState;
      previousBackwardColorState = config.backwardColorState;

      // Startup
      if(STARTUP_FORWARD){
        // Default to forward
        directionIsForward = true;
        forwardPixels.fill(COLORS[config.forwardColorState]);
        backwardPixels.fill(COLORS[config.backwardColorState]);
        previousColorForward = COLORS[config.forwardColorState];
        previousColorBackward = COLORS[config.backwardColorState];
      }else{
        directionIsForward = false;
        forwardPixels.fill(COLORS[config.backwardColorState]);
        backwardPixels.fill(COLORS[config.forwardColorState]);
        previousColorForward = COLORS[config.backwardColorState];
        previousColorBackward = COLORS[config.forwardColorState];
      }

    }

    void loop(uint8_t switchState, double erpm, double voltage){

      // Menu display
      if(config.pressCount > 0){
        if(config.longPressCount == 0){
          forwardPixels.clear();
          backwardPixels.clear();
          forwardPixels.setBrightness(BRIGHTNESS[config.brightnessState]);
          backwardPixels.setBrightness(BRIGHTNESS[config.brightnessState]);
          for(int i = 0; i < config.pressCount; i++){
            forwardPixels.setPixelColor(i, forwardPixels.Color(100, 100, 100));
            backwardPixels.setPixelColor(i, backwardPixels.Color(100, 100, 100));
            if(i == 6){
              forwardPixels.setPixelColor(i, forwardPixels.Color(0, 0, 0));
              backwardPixels.setPixelColor(i, backwardPixels.Color(0, 0, 0));  
            }
          }  
          forwardPixels.show();
          backwardPixels.show();
          return;
        }else if(config.pressCount == 1 || config.pressCount == 2 || config.pressCount == 3){
          if(config.pressCount == 3){
            directionIsForward = false;
          } else {
            directionIsForward = true;
          }
        }else if(config.pressCount == 4){
          forwardPixels.clear();
          backwardPixels.clear();
          forwardPixels.setBrightness(BRIGHTNESS[config.brightnessState]);
          backwardPixels.setBrightness(BRIGHTNESS[config.brightnessState]);
          for(int i = 0; i <= config.idleDisplayState; i++){
            forwardPixels.setPixelColor(i, forwardPixels.Color(0, 100, 0));
            backwardPixels.setPixelColor(i, backwardPixels.Color(0, 100, 0));
          }  
          forwardPixels.show();
          backwardPixels.show();
          return;
        }else if(config.pressCount == 5){
          forwardPixels.clear();
          backwardPixels.clear();
          forwardPixels.setBrightness(BRIGHTNESS[config.brightnessState]);
          backwardPixels.setBrightness(BRIGHTNESS[config.brightnessState]);
          int leds = config.batterySeriesState % 5;
          if(leds == 0){
            leds = 5;
          }
          for(int i = 0; i <= leds - 1; i++){
            forwardPixels.setPixelColor(i, forwardPixels.Color(0, 0, 100));
            backwardPixels.setPixelColor(i, backwardPixels.Color(0, 0, 100));
          }  
          forwardPixels.show();
          backwardPixels.show();
          return;
        }
      }

      //*****************
      //*Regular Display*
      //*****************
      
      // Detect direction, Latching behavior, if you know, you know.
      if(erpm > 25){
        directionIsForward = true;
      }else if(erpm < -25){
        directionIsForward = false;
      }

      // Update idle timer
      if(switchState != 0){
        idleTimer = millis();
      }

      if(millis() - idleTimer < IDLE_DELAY){
        isIdle = false;
        isIdleAndFaded = false;
      }else{
        isIdle = true;
      }

      // Disable fading when selecting color
      if(previousForwardColorState != config.forwardColorState || previousBackwardColorState != config.backwardColorState){
        previousForwardColorState = config.forwardColorState;
        previousBackwardColorState = config.backwardColorState;

        if(directionIsForward){
          previousColorForward = COLORS[config.forwardColorState];
          previousColorBackward = COLORS[config.backwardColorState];
        }else{
          previousColorForward = COLORS[config.backwardColorState];
          previousColorBackward = COLORS[config.forwardColorState];
        }
      }

      if(!isIdle || config.idleDisplayState == 0){
        // Regular display
        if(directionIsForward){
          fadeForwardColor(COLORS[config.forwardColorState], FADE_DIRECTION_SPEED);
          fadeBackwardColor(COLORS[config.backwardColorState], FADE_DIRECTION_SPEED);
        }else{
          fadeForwardColor(COLORS[config.backwardColorState], FADE_DIRECTION_SPEED);
          fadeBackwardColor(COLORS[config.forwardColorState], FADE_DIRECTION_SPEED);
        }
      }else if(isIdleAndFaded && config.idleDisplayState == 1){
        // Green/yellow/red battery display
        float batteryPercent = voltageTable.voltsToPercent5(voltage / config.batterySeriesState);
        uint32_t color =  forwardPixels.Color(255 * (1 - batteryPercent), 255 * batteryPercent, 0);
        fadeForwardColor(color, FADE_BATTERY_SPEED);
        fadeBackwardColor(color, FADE_BATTERY_SPEED);
      }else if(isIdleAndFaded && config.idleDisplayState == 2){
        // Green/yellow/red fade with percent illuminated.
        float batteryPercent = voltageTable.voltsToPercent100(voltage / config.batterySeriesState);
        uint32_t color =  forwardPixels.Color(255 * (1 - batteryPercent), 255 * batteryPercent, 0);
        fadeForwardColor(color, FADE_BATTERY_SPEED);
        fadeBackwardColor(color, FADE_BATTERY_SPEED);
        for(int i = roundf(batteryPercent * NUM_LEDS_FORWARD); i < NUM_LEDS_FORWARD; i++){
          forwardPixels.setPixelColor(i, forwardPixels.Color(0, 0, 0));
          backwardPixels.setPixelColor(i, backwardPixels.Color(0, 0, 0));  
        }
      }else if(isIdleAndFaded && config.idleDisplayState == 2){
        // Idle Off
        forwardPixels.clear();
        backwardPixels.clear();
      }

      if(isIdle){
        targetBrightness = IDLE_BRIGHTNESS[config.brightnessState];
      }else{
        targetBrightness = BRIGHTNESS[config.brightnessState];
      }
      

      // Fade Brightness
      if(previousBrightnessState != config.brightnessState){
        previousBrightnessState = config.brightnessState;
        forwardPixels.setBrightness(targetBrightness);
        backwardPixels.setBrightness(targetBrightness);
      }else{
        currentBrightness = forwardPixels.getBrightness();
        if(currentBrightness > targetBrightness){
          forwardPixels.setBrightness(max(currentBrightness - FADE_OUT_BY, targetBrightness));
          backwardPixels.setBrightness(max(currentBrightness - FADE_OUT_BY, targetBrightness));
        }else if(currentBrightness < targetBrightness){
          forwardPixels.setBrightness(min(currentBrightness + FADE_IN_BY, targetBrightness));
          backwardPixels.setBrightness(min(currentBrightness + FADE_IN_BY, targetBrightness));
        }else if (isIdle){
          isIdleAndFaded = true;
        }
      }
      
      forwardPixels.show();
      backwardPixels.show();
    }
};
