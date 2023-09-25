#ifndef _config
#define _config

#include <stdint.h>
#include <EEPROM.h>

// Color and brightness
#define DEFAULT_BRIGHTNESS_MODE 0
#define DEFAULT_COLOR_FORWARD_MODE 0
#define DEFAULT_COLOR_BACKWARD_MODE 5

#define BRIGHTNESS_MODES 4
#define COLOR_MODES 7

class Config {
  private:
    
  public:
    uint8_t brightnessState;

    uint8_t forwardColorState;
    uint8_t backwardColorState;
    
    void setup(){
      brightnessState = EEPROM.read(0);
      if(brightnessState > BRIGHTNESS_MODES - 1){
        brightnessState = DEFAULT_BRIGHTNESS_MODE;
      }

      forwardColorState = EEPROM.read(1);
      if(forwardColorState > COLOR_MODES - 1){
        forwardColorState = DEFAULT_COLOR_FORWARD_MODE;
      }

      backwardColorState = EEPROM.read(2);
      if(backwardColorState > COLOR_MODES - 1){
        backwardColorState = DEFAULT_COLOR_BACKWARD_MODE;
      }
    }
    void loop(){
      
    }

    void toggleBrightness(){
      brightnessState = (brightnessState + 1) % BRIGHTNESS_MODES;
      EEPROM.write(0, brightnessState);
    }

    void toggleForwardColor(){
      forwardColorState = (forwardColorState + 1) % COLOR_MODES;
      EEPROM.write(1, forwardColorState);
    }

    void toggleBackwardsColor(){
      backwardColorState = (backwardColorState + 1) % COLOR_MODES;
      EEPROM.write(2, backwardColorState);
    }

    
};

#endif
