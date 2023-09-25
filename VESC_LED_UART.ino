#include "config.cpp"
#include "buttons.cpp"
#include "balance_leds.cpp"
#include "./src/VescUart/src/VescUart.h"

Config config;
Buttons buttons(config);
BalanceLEDs balanceLEDs(config);
VescUart vesc;

void setup() {
  config.setup();
  buttons.setup();
  balanceLEDs.setup();
  
  Serial.begin(115200);
  vesc.setSerialPort(&Serial);
}

void loop() {
  vesc.getVescValues();
  vesc.getFloatValues();

  config.loop();
  buttons.loop(vesc.floatData.switchState, vesc.floatData.truePitch);
  balanceLEDs.loop(vesc.floatData.switchState, vesc.data.rpm, vesc.data.inpVoltage);

  delay(50); // Limit to 20hz
}
