#include "balance_leds.cpp"
#include "./src/VescUart/src/VescUart.h"

BalanceLEDs balanceLEDs;
VescUart vesc;

void setup() {
  balanceLEDs.setup();

    
  Serial.begin(115200);
  vesc.setSerialPort(&Serial);

}

void loop() {

  vesc.getVescValues();
  balanceLEDs.loop(vesc.data.rpm, vesc.data.inpVoltage);
  delay(50); // Limit to 20hz
}
