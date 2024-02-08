# VESC LED UART
Arduino based led controller board for BV's BLDC firmware. The main feture of this is directional head & tail lights.

## Features
1. Supports all LED features of the float package

## Parts List
1. Adruino Nano
1. 2x (or 3) LED Strips (SK6812 & WS2812)

# Configuration
Use the float package to configure the LEDs

## Options and pins
Pins & stuff are designed to be configured VIA the constants
1. VESC_LED_UART.ino: Configure UART freqency, you must match the value set in the VESC Tool
1. balance_leds.cpp: pin wiring.

## Compiling/Installing
All the required libraries are included, just hit the upload button in PlatformIO

