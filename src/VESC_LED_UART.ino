#include "balance_leds.cpp"
#include "./src/VescUart/src/VescUart.h"

BalanceLEDs balanceLEDs;
VescUart vesc;

void setup() {
  Serial.begin(115200);
  vesc.setSerialPort(&Serial);
  balanceLEDs.setup();

  // Test data
//  vesc.data.rpm = 290;
//  vesc.data.dutyCycleNow = 0.3; 
//  vesc.floatData.led_type = 2;
//  vesc.floatData.led_rear_count = 8;
//  vesc.floatData.led_forward_count = 8;
//  vesc.floatData.led_status_count = 0;
//  vesc.floatData.led_status_brightness = 10;
//  vesc.floatData.led_brightness = 1;
//  vesc.floatData.led_brightness_idle = 3;
//  vesc.floatData.led_mode = 3;
//  vesc.floatData.led_mode_idle = 5;
//  vesc.floatData.switchState = 1;
//  vesc.floatData.batteryPercent = 1.0;
//  vesc.floatData.state = 5;
}

int slowUpdate = 0;
int fastUpdate = 0;
void loop() {
  long currentMillis = millis();
  if(currentMillis / 2010 != slowUpdate){
    slowUpdate = currentMillis / 2010;
    vesc.getFloatBattery();
    vesc.getFloatLeds();
  }
  if(currentMillis / 300 != fastUpdate){
    fastUpdate = currentMillis / 300;  
    vesc.getVescValues();
    vesc.getFloatValues();
  }

  balanceLEDs.led_update_before(&vesc.data, &vesc.floatData);
  balanceLEDs.led_update();
  balanceLEDs.led_update_after();

  delay(1);
}
