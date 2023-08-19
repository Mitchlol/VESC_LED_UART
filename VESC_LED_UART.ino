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
  //balanceLEDs.loop(vesc.data.rpm, vesc.data.inpVoltage);
  delay(50); // Limit to 20hz

  if(millis() % 20000 < 4000){
    balanceLEDs.loop(100, 84); 
  }else if(millis() % 20000 < 8000){
    balanceLEDs.loop(-100, 84);
  }else if(millis() % 20000 < 16000){
    balanceLEDs.loop(0, 84);
  }else{
    balanceLEDs.loop(0, 70);
  }
}
