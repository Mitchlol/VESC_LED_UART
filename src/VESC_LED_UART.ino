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
//  vesc.floatData.led_type = 1;
//  vesc.floatData.led_rear_count = 4;
//  vesc.floatData.led_forward_count = 4;
//  vesc.floatData.led_status_count = 4;
//  vesc.floatData.led_status_brightness = 10;
//  vesc.floatData.led_brightness = 10;
//  vesc.floatData.led_brightness_idle = 10;
//  vesc.floatData.led_mode = 1;
//  vesc.floatData.led_mode_idle = 9;
//  vesc.floatData.switchState = 1;
//  vesc.floatData.batteryPercent = 1.0;
//  vesc.floatData.state = 5;
}

void loop() {
  vesc.getVescValues();
  vesc.getFloatValues();
  vesc.getFloatBattery();
  vesc.getFloatLeds();

  balanceLEDs.led_update_before(&vesc.data, &vesc.floatData);
  balanceLEDs.led_update();
  balanceLEDs.led_update_after();

  
}
