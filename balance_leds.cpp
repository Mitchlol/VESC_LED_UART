#define FASTLED_INTERNAL
#include "./src/Adafruit_NeoPixel/Adafruit_NeoPixel.h"

// LED Type
#define LED_TYPE NEO_GRB

// LED Wiring
// Note: Forward in this instance means postive ERPM, which can actually be backwards if your motor direction is reversed.
#define LED_PIN_FORWARD 10
#define LED_PIN_BACKWARD 11
#define NUM_LEDS_FORWARD 10
#define NUM_LEDS_BACKWARD 10
#define STARTUP_FORWARD true

// Color and brightness
#define COLOR_FORWARD 0xFFFFFFFF
#define COLOR_BACKWARD 0x00FF0000
#define BRIGHTNESS 255
#define IDLE_BRIGHTNESS 10

// Timings
#define IDLE_DELAY 1000
#define FADE_OUT_BY 10
#define FADE_IN_BY 35
#define FADE_DIRECTION_SPEED 20
#define FADE_BATTERY_SPEED 20

// Battery Meter (li-ion only)
#define BATTERY_SERIES_COUNT 20
#define BATTERY_DISPLAY_MODE 1


class BalanceLEDs {
  private:

    // Runtime vars
    bool directionIsForward;
    uint8_t currentBrightness;
    uint8_t targetBrightness;
    long idleTimer;
    bool isIdle;
    bool isIdleAndFaded;
    uint32_t previousColorForward;
    uint32_t previousColorBackward;

    Adafruit_NeoPixel forwardPixels{NUM_LEDS_FORWARD, LED_PIN_FORWARD, LED_TYPE + NEO_KHZ800};
    Adafruit_NeoPixel backwardPixels{NUM_LEDS_BACKWARD, LED_PIN_BACKWARD, LED_TYPE + NEO_KHZ800};


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

    // Really basic linear voltage to percent calculation
    float getBatteryPercent(float batteryVoltage){
      float cellVoltage = batteryVoltage / BATTERY_SERIES_COUNT;
      if(cellVoltage < 3){
        return 0;
      }else if(cellVoltage > 4.2){
        return 1;
      }else{
        return (cellVoltage - 3) * 0.833333;
      } 
    }
  public:
    
    void setup(){

      // Init LEDs
      forwardPixels.begin();
      backwardPixels.begin();
      forwardPixels.setBrightness(BRIGHTNESS);
      backwardPixels.setBrightness(BRIGHTNESS);

      // Startup
      if(STARTUP_FORWARD){
        // Default to forward
        directionIsForward = true;
        forwardPixels.fill(COLOR_FORWARD);
        backwardPixels.fill(COLOR_BACKWARD);
        previousColorForward = COLOR_FORWARD;
        previousColorBackward = COLOR_BACKWARD;
      }else{
        directionIsForward = false;
        forwardPixels.fill(COLOR_BACKWARD);
        backwardPixels.fill(COLOR_FORWARD);
        previousColorForward = COLOR_BACKWARD;
        previousColorBackward = COLOR_FORWARD;
      }

    }

    void loop(double erpm, double voltage){
      // Detect direction, Latching behavior, if you know, you know.
      if(erpm > 10){
        directionIsForward = true;
      }else if(erpm < -10){
        directionIsForward = false;
      }

      // Update idle timer
      if(erpm >= 5 || erpm <= -5){
        idleTimer = millis();
      }

      if(millis() - idleTimer < IDLE_DELAY){
        isIdle = false;
        isIdleAndFaded = false;
      }else{
        isIdle = true;
      }

      if(!isIdle || BATTERY_DISPLAY_MODE == 0){
        // Regular display
        if(directionIsForward){
          fadeForwardColor(COLOR_FORWARD, FADE_DIRECTION_SPEED);
          fadeBackwardColor(COLOR_BACKWARD, FADE_DIRECTION_SPEED);
        }else{
          fadeForwardColor(COLOR_BACKWARD, FADE_DIRECTION_SPEED);
          fadeBackwardColor(COLOR_FORWARD, FADE_DIRECTION_SPEED);
        }
      }else if(isIdleAndFaded && BATTERY_DISPLAY_MODE == 1){
        // Green/yellow/red battery display
        float batteryPercent = getBatteryPercent(voltage);
        uint32_t color =  forwardPixels.Color(255 * (1 - batteryPercent), 255 * batteryPercent, 0);
        fadeForwardColor(color, FADE_BATTERY_SPEED);
        fadeBackwardColor(color, FADE_BATTERY_SPEED);
      }

      if(isIdle){
        targetBrightness = IDLE_BRIGHTNESS;
      }else{
        targetBrightness = BRIGHTNESS;
      }

      

      // Fade Brightness
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
      
      forwardPixels.show();
      backwardPixels.show();
    }
};
