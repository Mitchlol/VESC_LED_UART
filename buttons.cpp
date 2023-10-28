#include <stdint.h>
#include <Arduino.h>
#include "config.cpp"

class Buttons {
  private:
    Config& config;
    long timer;
    int previousSwitchState;
 
  public:
    Buttons(Config& mConfig): config(mConfig) {}  
    void setup(){
      
    }
    void loop(uint8_t switchState, float pitch){
      // Block input when angle is invalid
      if(pitch < 70 || pitch > 110){
        previousSwitchState = 0;
        config.pressCount = 0;
        config.longPressCount = 0;
        return;
      }

      // Clear press count & state when idle
      if(millis() - timer > 1000){
        previousSwitchState = 0;
        config.pressCount = 0;
        config.longPressCount = 0;
      }

      // Press detected but we dont know what it is yet
      if(previousSwitchState == 0 && switchState == 1){
        previousSwitchState = 1;
        timer = millis();
      }

      // Single press confirmed upon release, increment counter
      if(previousSwitchState == 1 && switchState == 0){
        previousSwitchState = 0;
        config.pressCount ++;
        timer = millis();
      }

      // Keep togging brightness on a dual hold after a single press
      if(config.pressCount == 1 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.longPressCount++;
        config.toggleBrightness();
      }else if(config.pressCount == 1 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.longPressCount++;
        config.toggleBrightness();
      }

      // Forward color
      if(config.pressCount == 2 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.longPressCount++;
        config.toggleForwardColor();
      }else if(config.pressCount == 2 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.longPressCount++;
        config.toggleForwardColor();
      }

      // Backwards color
      if(config.pressCount == 3 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.longPressCount++;
        config.toggleBackwardsColor();
      }else if(config.pressCount == 3 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.longPressCount++;
        config.toggleBackwardsColor();
      }

      //Voltage Display Mode
      if(config.pressCount == 4 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.longPressCount++;
        config.toggleIdle();
      }else if(config.pressCount == 4 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.longPressCount++;
        config.toggleIdle();
      }

      // Cell count
      if(config.pressCount == 5 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.longPressCount++;
        config.setBatterySeries(1);
      }else if(config.pressCount == 5 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.longPressCount++;
        config.setBatterySeries(config.batterySeriesState + 1);
      }

      // LED count
      if(config.pressCount == 6 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.longPressCount++;
        config.setLedCount(1);
      }else if(config.pressCount == 6 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.longPressCount++;
        config.setLedCount(config.ledCountState + 1);
      }
      
      // LED Type
      if(config.pressCount == 7 && switchState == 2 && previousSwitchState != 2){
        timer = millis();
        previousSwitchState = 2;
        config.longPressCount++;
        config.toggleLedType();
      }else if(config.pressCount == 7 && switchState == 2 && millis() - timer > 500){
        timer = millis();
        config.longPressCount++;
        config.toggleLedType();
      }
    }
};
