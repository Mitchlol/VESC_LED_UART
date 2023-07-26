# VESC LED UART
Arduino based led controller board for BV's BLDC firmware. The main feture of this is directional head & tail lights.

## Features
1. Directional LEDs for headlight/taillight with dimming.
1. Voltage display when idle.
1. Simple & reliable uart connection.

## Parts List
1. Adruino Nano
1. 2x LED Strips (WS2812 GRB is the default, but anything supported by FastLED should work)

# Configuration
## Options and pins
Features are designed to be configured VIA the constants
1. VESC_LED_UART.ino: Configure UART freqency, you must match the value set in the VESC Tool
1. balance_leds.cpp: Configure wiritng, LED Type, Brightnes, etc.

## Compiling/Installing
All the required libraries are included, just hit the upload button in Arduino IDE

