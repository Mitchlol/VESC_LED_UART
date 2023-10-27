#ifndef _config
#define _config

#include <stdint.h>
#include <EEPROM.h>

// Color and brightness
#define DEFAULT_BRIGHTNESS_MODE 0
#define DEFAULT_COLOR_FORWARD_MODE 0
#define DEFAULT_COLOR_BACKWARD_MODE 5

// Battery
#define DEFAULT_IDLE_DISPLAY_MODE 1
#define DEFAULT_BATTERY_SERIES_COUNT 20

#define BRIGHTNESS_MODES 4
#define COLOR_MODES 7
#define IDLE_MODES 4

class Config {
  private:
    
  public:
    uint8_t brightnessState;
    uint8_t forwardColorState;
    uint8_t backwardColorState;
    uint8_t idleDisplayState;
    uint8_t batterySeriesState;

    uint8_t pressCount;
    uint8_t longPressCount;
    
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

      idleDisplayState = EEPROM.read(3);
      if(idleDisplayState > 2){
        idleDisplayState = DEFAULT_IDLE_DISPLAY_MODE;
      }

      batterySeriesState = EEPROM.read(4);
      if(batterySeriesState > 40){
        batterySeriesState = DEFAULT_BATTERY_SERIES_COUNT;
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

    void toggleIdle(){
      idleDisplayState = (idleDisplayState + 1) % IDLE_MODES;
      EEPROM.write(3, idleDisplayState);
    }

    void setBatteySeries(int newValue){
      batterySeriesState = newValue;
      EEPROM.write(4, batterySeriesState);
    }

    
};

#endif
