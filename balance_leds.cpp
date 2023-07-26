#define FASTLED_INTERNAL
#include "./src/FastLED/src/FastLED.h"

// LED Type
#define LED_TYPE WS2812
#define COLOR_ORDER GRB

// LED Wiring
// Note: Forward in this instance means postive ERPM, which can actually be backwards if your motor direction is reversed.
#define LED_PIN_FOREWARD 10
#define LED_PIN_BACKWARD 11
#define NUM_LEDS_FORWARD 10
#define NUM_LEDS_BACKWARD 10
#define STARTUP_FORWARD true

// Color and brightness
// For full color list see http://fastled.io/docs/3.1/struct_c_r_g_b.html
#define COLOR_FORWARD CRGB::White
#define COLOR_BACKWARD CRGB::Red
#define BRIGHTNESS 255
#define IDLE_BRIGHTNESS 10

// Timings
#define IDLE_DELAY 1000
#define FADE_OUT_BY 10
#define FADE_IN_BY 35
#define FADE_DIRECTION_SPEED 75
#define FADE_BATTERY_SPEED 75

// Battery Meter (li-ion only)
#define BATTERY_SERIES_COUNT 20
#define BATTERY_DISPLAY_MODE 1


class BalanceLEDs {
  private:

    // Runtime vars
    bool directionIsForward;
    uint8_t targetBrightness;
    long idleTimer;
    bool isIdle;
    bool isIdleAndFaded;
  
    CRGB forward[NUM_LEDS_FORWARD];
    CRGB backward[NUM_LEDS_BACKWARD];

    // Helper function that blends one uint8_t toward another by a given amount
    void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount){
      if( cur == target) return;
      
      if( cur < target ) {
        uint8_t delta = target - cur;
        delta = scale8_video( delta, amount);
        cur += delta;
      } else {
        uint8_t delta = cur - target;
        delta = scale8_video( delta, amount);
        cur -= delta;
      }
    }

    // Blend one CRGB color toward another CRGB color by a given amount.
    // Blending is linear, and done in the RGB color space.
    // This function modifies 'cur' in place.
    CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount){
      nblendU8TowardU8( cur.red,   target.red,   amount);
      nblendU8TowardU8( cur.green, target.green, amount);
      nblendU8TowardU8( cur.blue,  target.blue,  amount);
      return cur;
    }

    // Fade an entire array of CRGBs toward a given background color by a given amount
    // This function modifies the pixel array in place.
    void fadeTowardColor( CRGB* L, uint16_t N, const CRGB& bgColor, uint8_t fadeAmount){
      for( uint16_t i = 0; i < N; i++) {
        fadeTowardColor( L[i], bgColor, fadeAmount);
      }
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

      // Configure LEDs
      FastLED.addLeds<LED_TYPE, LED_PIN_FOREWARD, COLOR_ORDER>(forward, NUM_LEDS_FORWARD).setCorrection( TypicalLEDStrip );
      FastLED.addLeds<LED_TYPE, LED_PIN_BACKWARD, COLOR_ORDER>(backward, NUM_LEDS_BACKWARD).setCorrection( TypicalLEDStrip );
      FastLED.setBrightness(0.0);

      // Startup
      if(STARTUP_FORWARD){
        // Default to forward
        directionIsForward = true;
        fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_FORWARD, FADE_DIRECTION_SPEED);
        fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_BACKWARD, FADE_DIRECTION_SPEED); 
      }else{
        directionIsForward = false;
        fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_BACKWARD, FADE_DIRECTION_SPEED);
        fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_FORWARD, FADE_DIRECTION_SPEED);
      }
      FastLED.show();
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
          fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_FORWARD, FADE_DIRECTION_SPEED);
          fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_BACKWARD, FADE_DIRECTION_SPEED);
        }else{
          fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_BACKWARD, FADE_DIRECTION_SPEED);
          fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_FORWARD, FADE_DIRECTION_SPEED);
        }
      }else if(isIdleAndFaded && BATTERY_DISPLAY_MODE == 1){
        // Green/yellow/red battery display
        float batteryPercent = getBatteryPercent(voltage);
        CRGB color;
        color.red = 255 * (1 - batteryPercent);
        color.green = 255 * batteryPercent;
        color.blue = 0;
        fadeTowardColor(forward, NUM_LEDS_FORWARD, color, FADE_BATTERY_SPEED);
        fadeTowardColor(backward, NUM_LEDS_BACKWARD, color, FADE_BATTERY_SPEED);
      }

      if(isIdle){
        targetBrightness = IDLE_BRIGHTNESS;
      }else{
        targetBrightness = BRIGHTNESS;
      }

      

      // Fade Brightness
      if(FastLED.getBrightness() > targetBrightness){
        FastLED.setBrightness(max(FastLED.getBrightness() - FADE_OUT_BY, targetBrightness));
      }else if(FastLED.getBrightness() < targetBrightness){
        FastLED.setBrightness(min(FastLED.getBrightness() + FADE_IN_BY, targetBrightness));
      }else if (isIdle){
        isIdleAndFaded = true;
      }

      FastLED.show();
    }
};
