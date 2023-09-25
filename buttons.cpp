#include <stdint.h>
#include <Arduino.h>
#include "config.cpp"

class Buttons {
  private:
    Config& config;
    long timer;
    int previousSwitchState;
    int pressCount;
 
  public:
    Buttons(Config& mConfig): config(mConfig) {}  
    void setup(){
      
    }
    void loop(uint8_t switchState, float pitch){

      // Clear press count & state when idle
      if(millis() - timer > 1000 || pitch < 70 || pitch > 110){
        previousSwitchState = 0;
        pressCount = 0;
      }

      // Press detected but we dont know what it ise yet
      if(previousSwitchState == 0 && switchState == 1){
        previousSwitchState = 1;
        timer = millis();
      }

      // Single press confirmed upon release, increment counter
      if(previousSwitchState == 1 && switchState == 0){
        previousSwitchState = 0;
        pressCount ++;
        timer = millis();
      }

      // Keep togging brightness on a dual hold after a single press
      if(pressCount == 1 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.toggleBrightness();
      }else if(pressCount == 1 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.toggleBrightness();
      }

      // Forward color
      if(pressCount == 2 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.toggleForwardColor();
      }else if(pressCount == 2 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.toggleForwardColor();
      }

      // Backwards color
      if(pressCount == 3 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.toggleBackwardsColor();
      }else if(pressCount == 3 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.toggleBackwardsColor();
      }

    }
};
