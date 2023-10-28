# VESC LED UART
Arduino based led controller board for BV's BLDC firmware. The main feture of this is directional head & tail lights.

## Features
1. Directional LEDs for headlight/taillight with dimming.
1. Voltage display when idle.
1. Simple & reliable uart connection.
1. Configurable color & brightness via footpad UI
1. Configurable cell count & led type via footpad UI

## Parts List
1. Adruino Nano
1. 2x LED Strips (SK6812 & WS2812)

# Configuration
## Footpad UI
All options can be set via footpad presses, to start the board must be nose up, then a single pad press will increment through the menu, and a dual pad hold will scroll through the options in that menu.
1. Brightness, 4 levels including off (affects both on and idle brightness)
1. Front color, 7 options (white, blue, cyan, green, yellow, red, magenta)
1. Rear color, 7 options (same as front, sets direction to reverse so you can see it)
1. Idle mode, 4 options (Regular colors, battery green->red fade in 5 levels, battery percent in green->red fade and leds on/off, leds fully off)
1. Cell count, hold to counts up to your cell count, release to set (UI only shows 5 leds at a time, so for 18s hold past 5 for 3 loops, and then 3 more illuminated)
1. LED count, same as cell count, except reboot is required to apply.
1. LED type, 2 options, reboot is required to apply (1 = sk6812 aka rgbw, 2 = ws2812 rgb) 

## Options and pins
Features are designed to be configured VIA the constants
1. VESC_LED_UART.ino: Configure UART freqency, you must match the value set in the VESC Tool
1. balance_leds.cpp: Brightness & color options etc.
1. config.cpp: Default values for battery and led configurations.

## Compiling/Installing
All the required libraries are included, just hit the upload button in Arduino IDE (v1)

