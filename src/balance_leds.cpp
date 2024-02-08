#define FASTLED_INTERNAL
#include "./src/Adafruit_NeoPixel/Adafruit_NeoPixel.h"
#include "./src/VescUart/src/VescUart.h"

// LED Wiring
// Note: Forward in this instance means postive ERPM, which can actually be backwards if your motor direction is reversed.
#define LED_PIN_STATUS 12
#define LED_PIN_FORWARD 10
#define LED_PIN_BACKWARD 11

class BalanceLEDs {
  private:
    Adafruit_NeoPixel *statusPixels;
    Adafruit_NeoPixel *forwardPixels;
    Adafruit_NeoPixel *backwardPixels;

    // New Vars
    typedef enum {
      LED_Type_None,
      LED_Type_RGB,
      LED_Type_RGBW,
      LED_Type_External_Module,
    } LEDType;

    struct LEDData {
      float led_last_updated;
      uint8_t led_previous_brightness;
      bool led_latching_direction;
      uint8_t led_type;
      uint8_t led_status_count;
      uint8_t led_forward_count;
      uint8_t led_rear_count;
      int ledbuf_len;
      uint32_t* RGBdata;
    };

    struct float_config {
      uint8_t led_brightness;
      uint8_t led_brightness_idle;
      uint8_t led_mode;
      uint8_t led_mode_idle;
      uint8_t led_status_brightness;
      uint8_t led_status_mode;
      uint16_t fault_adc_half_erpm;
    };

    LEDData* led_data;
    float_config* float_conf; 
    float current_time;
    float erpm; 
    float abs_duty_cycle;
    int switch_state; 
    int float_state;
    float batteryLevel;


  public:
    
    void setup(){
      // Create objects
      led_data = (LEDData *) malloc(sizeof(LEDData));
      float_conf = (float_config *) malloc(sizeof(float_config));

      //Set Defaults
      led_data->led_last_updated = 0;
      led_data->led_previous_brightness = 0;
      led_data->led_latching_direction = false;
      led_data->led_type = 0;
      led_data->led_status_count = 0;
      led_data->led_forward_count = 0;
      led_data->led_rear_count = 0;
      
      // TODO: Get real value somehow
      float_conf->fault_adc_half_erpm = 300;

      statusPixels = new Adafruit_NeoPixel{};
      forwardPixels = new Adafruit_NeoPixel{};
      backwardPixels = new Adafruit_NeoPixel{};
      led_data->RGBdata = (uint32_t*)malloc(0);
    }

    void populate_clone_objects(VescUart::dataPackage * dataData, VescUart::floatPackage* floatData){
      // LED Data Object
      // NOOP: led_data->led_last_updated;
      // NOOP: led_data->led_previous_brightness;
      // NOOP: led_data->led_latching_direction;

      if(led_data->led_type != floatData->led_type  ||
        led_data->led_status_count != floatData->led_status_count ||
        led_data->led_forward_count != floatData->led_forward_count ||
        led_data->led_rear_count != floatData->led_rear_count
      ){
        led_data->led_type = floatData->led_type;

        // Strips are actually GRB but i coppied vesc logic that flips it, and want to make minimal edits to the vesc code
        // So we flip it here to unflip it by setting the type to RGB instead of GRB
        unsigned int pixelType = NEO_WRGB;
        if(led_data->led_type == 1){
          pixelType = NEO_RGB;
        }

        statusPixels->clear();
        statusPixels->show();
        led_data->led_status_count = floatData->led_status_count;
        if(led_data->led_status_count > 0){
            statusPixels = new Adafruit_NeoPixel{led_data->led_status_count, LED_PIN_STATUS, pixelType + NEO_KHZ800};
            statusPixels->begin();
            statusPixels->setBrightness(255); 
        }

        forwardPixels->clear();
        forwardPixels->show();
        led_data->led_forward_count = floatData->led_forward_count;
        if(led_data->led_forward_count > 0){
            forwardPixels = new Adafruit_NeoPixel{led_data->led_forward_count, LED_PIN_FORWARD, pixelType + NEO_KHZ800};
            forwardPixels->begin();
            forwardPixels->setBrightness(255); 
        }

        backwardPixels->clear();
        backwardPixels->show();
        led_data->led_rear_count = floatData->led_rear_count;
        if(led_data->led_rear_count > 0){
            backwardPixels = new Adafruit_NeoPixel{led_data->led_rear_count, LED_PIN_BACKWARD, pixelType + NEO_KHZ800};
            backwardPixels->begin();
            backwardPixels->setBrightness(255);
        }

        led_data->ledbuf_len = led_data->led_status_count + led_data->led_forward_count + led_data->led_rear_count + 1;
        free(led_data->RGBdata);
        led_data->RGBdata = (uint32_t*)malloc(sizeof(uint32_t) * led_data->ledbuf_len);
        for(int i = 0; i < led_data->ledbuf_len; i++){
          led_data->RGBdata[i] = 0;
        }
      }

      // Float Conf Object
      float_conf->led_brightness = floatData->led_brightness;
      float_conf->led_brightness_idle = floatData->led_brightness_idle;
      float_conf->led_mode = floatData->led_mode;
      float_conf->led_mode_idle = floatData->led_mode_idle;
      float_conf->led_status_brightness = floatData->led_status_brightness;
      float_conf->led_status_mode = floatData->led_status_mode;


      // Misc vars
      current_time = millis()/1000.0f;
      erpm = dataData->rpm; 
      abs_duty_cycle = abs(dataData->dutyCycleNow);
      switch_state = floatData->switchState; 
      float_state = floatData->state;
      batteryLevel = floatData->batteryPercent; 
    }


    uint32_t led_fade_color(uint32_t from, uint32_t to) {
        uint32_t fw = (from >> 24) & 0xFF;
        uint32_t fr = (from >> 16) & 0xFF;
        uint32_t fg = (from >> 8) & 0xFF;
        uint32_t fb = from & 0xFF;
    
        uint8_t tw = (to >> 24) & 0xFF;
        uint8_t tr = (to >> 16) & 0xFF;
        uint8_t tg = (to >> 8) & 0xFF;
        uint8_t tb = to & 0xFF;
    
        if (fw < tw) {
            if (fw + 12 > tw) {
                fw = tw;
            } else {
                fw += 12;
            }
        } else if (fw > tw) {
            if (fw - 12 < tw) {
                fw = tw;
            } else {
                fw -= 12;
            }
        }
        if (fr < tr) {
            if (fr + 12 > tr) {
                fr = tr;
            } else {
                fr += 12;
            }
        } else if (fr > tr) {
            if (fr - 12 < tr) {
                fr = tr;
            } else {
                fr -= 12;
            }
        }
        if (fg < tg) {
            if (fg + 12 > tg) {
                fg = tg;
            } else {
                fg += 12;
            }
        } else if (fg > tg) {
            if (fg - 12 < tg) {
                fg = tg;
            } else {
                fg -= 12;
            }
        }
        if (fb < tb) {
            if (fb + 12 > tb) {
                fb = tb;
            } else {
                fb += 12;
            }
        } else if (fb > tb) {
            if (fb - 12 < tb) {
                fb = tb;
            } else {
                fb -= 12;
            }
        }
        return (fw << 24) | (fr << 16) | (fg << 8) | fb;
    }

    
    uint32_t led_rgb_to_local(uint32_t color, uint8_t brightness, bool rgbw) {
        uint32_t w = (color >> 24) & 0xFF;
        uint32_t r = (color >> 16) & 0xFF;
        uint32_t g = (color >> 8) & 0xFF;
        uint32_t b = color & 0xFF;
    
        r = (r * brightness) / 100;
        g = (g * brightness) / 100;
        b = (b * brightness) / 100;
        w = (w * brightness) / 100;
    
        if (rgbw) {
            return (g << 24) | (r << 16) | (b << 8) | w;
        } else {
            return (g << 16) | (r << 8) | b;
        }
    }

    void led_set_color(LEDData* led_data, int led, uint32_t color, uint32_t brightness, bool fade) {
        if ((led_data->led_type == LED_Type_None) || (led_data->led_type == LED_Type_External_Module)) {
            return;
        }
        if (led >= 0 && led < led_data->ledbuf_len) {
            if (fade) {
                color = led_fade_color(led_data->RGBdata[led], color);
            }
            led_data->RGBdata[led] = color;
    
            color = led_rgb_to_local(color, brightness, led_data->led_type == 2);

            if(led < led_data->led_status_count){
              statusPixels->setPixelColor(led, color);
            }else if(led < led_data->led_status_count + led_data->led_forward_count){
              forwardPixels->setPixelColor(led - led_data->led_status_count, color);
            }else{
              backwardPixels->setPixelColor(led - led_data->led_status_count - led_data->led_forward_count, color);
            }
        }
    }

    void led_strip_set_color(LEDData* led_data, int offset, int length, uint32_t color, uint32_t brightness, bool fade) {
        for(int led= offset; led < offset+length; led++){
            led_set_color(led_data, led, color, brightness, fade);
        }
    }

    void led_display_battery(LEDData* led_data, int brightness, int strip_offset, int strip_length, bool fade) {
        int batteryLeds = (int)(batteryLevel * strip_length);
        uint32_t batteryColor = 0x0000FF00;
        if (batteryLevel < .4) {
            batteryColor = 0x00FFFF00;
        }
        if (batteryLevel < .2) {
            batteryColor = 0x00FF0000;
        }
        for (int i = strip_offset; i < strip_offset + strip_length; i++) {
            if (i < strip_offset + batteryLeds) {
                led_set_color(led_data, i, batteryColor, brightness, fade);
            } else {
                led_set_color(led_data, i, 0x00000000, brightness, fade);
            }
        }
    }

    void led_float_disabled(LEDData* led_data, int brightness, int first, int count) {
        // show red LEDs in the center of the light bar
        int start = count / 4;
        int end = count * 3 / 4;
        if (end < 3) { // 3 or fewer LEDs in total? Show red lights across the entire light bar
            end = count;
        }
        for (int i = first; i < first + count; i++) {
            int bright = brightness;
            if ((end > 4) && ((i == first+start) || (i == first+end-1))) // first and last red led at half brightness
                bright /= 2;
            if ((i < first+start) || (i > first+end)) // outer LEDs are off
                bright = 0;
            led_set_color(led_data, i, 0x00FF0000, bright, false);
        }
    }

    void led_update_before(VescUart::dataPackage * dataData, VescUart::floatPackage* floatData) {
      populate_clone_objects(dataData, floatData);
    }

    void led_update() {      
      ///////////////////////
      // Status LED Logic //
      /////////////////////
      if ((led_data->led_type == LED_Type_None) || (led_data->led_type == LED_Type_External_Module)) {
          return;
      }
      if (current_time - led_data->led_last_updated < 0.05) {
          return;
      }
      led_data->led_last_updated = current_time;
  
      if (led_data->led_status_count > 0) {
          int statusBrightness = (int)(float_conf->led_status_brightness);
          if (float_state == 15) {
              led_float_disabled(led_data, statusBrightness, 0, led_data->led_status_count);
          }
          else if (fabsf(erpm) < float_conf->fault_adc_half_erpm) {
              // Display status LEDs
              if (switch_state == 0) {
                  led_display_battery(led_data, statusBrightness, 0, led_data->led_status_count, false);
              } else if (switch_state == 1) {
                  for (int i = 0; i < led_data->led_status_count; i++) {
                      if (i < led_data->led_status_count / 2) {
                          led_set_color(led_data, i, 0x000000FF, statusBrightness, false);
                      } else {
                          led_set_color(led_data, i, 0x00000000, 0, false);
                      }
                  }
              } else {
                  for (int i = 0; i < led_data->led_status_count; i++) {
                      led_set_color(led_data, i, 0x000000FF, statusBrightness, false);
                  }
              }
          } else {
              // Display duty cycle when riding
              int dutyLeds = (int)(fminf((abs_duty_cycle * 1.1112), 1) * led_data->led_status_count);
              uint32_t dutyColor = 0x00FFFF00;
              if (abs_duty_cycle > 0.85) {
                  dutyColor = 0x00FF0000;
              } else if (abs_duty_cycle > 0.7) {
                  dutyColor = 0x00FF8800;
              }
  
              for (int i = 0; i < led_data->led_status_count; i++) {
                  if (i < dutyLeds) {
                      led_set_color(led_data, i, dutyColor, statusBrightness, false);
                  } else {
                      led_set_color(led_data, i, 0x00000000, 0, false);
                  }
              }
          }
      }
  
      int brightness = float_conf->led_brightness;
      if (float_state > 5) {
          // board is disengaged
          brightness = float_conf->led_brightness_idle;
      }
      if (brightness > led_data->led_previous_brightness) {
          led_data->led_previous_brightness += 5;
          if (led_data->led_previous_brightness > brightness) {
              led_data->led_previous_brightness = brightness;
          }
      } else if (brightness < led_data->led_previous_brightness) {
          led_data->led_previous_brightness -= 5;
          if (led_data->led_previous_brightness < brightness) {
              led_data->led_previous_brightness = brightness;
          }
      }
      brightness = led_data->led_previous_brightness;
  
      /////////////////////////
      // Patten strip logic //
      ///////////////////////
  
      // calculate directonality
      int forwardOffsetDirectional = led_data->led_status_count;
      int forwardlengthDirectional = led_data->led_forward_count;
      int rearOffsetDirectional = led_data->led_status_count + led_data->led_forward_count;
      int rearlengthDirectional = led_data->led_rear_count;
      if (float_state == 4) { // RUNNING_UPSIDEDOWN aka drark ride => flip erpm sign
          erpm = -erpm;
      }
  
      if (erpm > 100) {
          led_data->led_latching_direction = true;
      } else if (erpm < -100) {
          led_data->led_latching_direction = false;
      }
      if (led_data->led_latching_direction == false) {
          forwardOffsetDirectional = led_data->led_status_count + led_data->led_forward_count;
          forwardlengthDirectional = led_data->led_rear_count;
          rearOffsetDirectional = led_data->led_status_count;
          rearlengthDirectional = led_data->led_forward_count;
      }
  
      // Find color
      int led_mode = float_state < 5 ? float_conf->led_mode : float_conf->led_mode_idle;
      if (float_state == 15) { // disabled board? front leds show red in center, rear leds off
          led_float_disabled(led_data, brightness, led_data->led_status_count, led_data->led_forward_count);
          led_strip_set_color(led_data, led_data->led_status_count + led_data->led_forward_count, led_data->led_rear_count, 0, 0, true);
          return;
      }else if (led_mode == 0) { // Red/White
          led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0xFFFFFFFF, brightness, true);
          led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FF0000, brightness, true);
          return;
      } else if (led_mode == 1) { //Battery Meter
          led_display_battery(led_data, brightness, led_data->led_status_count, led_data->led_forward_count, true);
          led_display_battery(led_data, brightness, led_data->led_status_count + led_data->led_forward_count, led_data->led_rear_count, true);
          return;
      } else if (led_mode == 2) { // Cyan/Magenta
          led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x0000FFFF, brightness, true);
          led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FF00FF, brightness, true);
          return;
      } else if (led_mode == 3) { // Blue/Green
          led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x000000FF, brightness, true);
          led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x0000FF00, brightness, true);
          return;
      } else if (led_mode == 4) { // Yellow/Green
          led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x00FFFF00, brightness, true);
          led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x0000FF00, brightness, true);
          return;
      } else if (led_mode == 5) { // RGB Fade
          if ((uint32_t)(current_time * 1000) % 3000 < 1000) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x00FF0000, brightness, true);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FF0000, brightness, true);
          } else if ((uint32_t)(current_time * 1000) % 3000 < 2000) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x0000FF00, brightness, true);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x0000FF00, brightness, true);
          } else {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x000000FF, brightness, true);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x000000FF, brightness, true);
          }
          return;
      } else if (led_mode == 6) { // Strobe
          if (led_data->RGBdata[forwardOffsetDirectional] > 0) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x00000000, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00000000, brightness, false);
          } else {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0xFFFFFFFF, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0xFFFFFFFF, brightness, false);
          }
          return;
      } else if (led_mode == 7) { // Rave
          if (led_data->RGBdata[forwardOffsetDirectional] == 0x00FF0000) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x00FFFF00, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FFFF00, brightness, false);
          } else if (led_data->RGBdata[forwardOffsetDirectional] == 0x00FFFF00) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x0000FF00, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x0000FF00, brightness, false);
          } else if (led_data->RGBdata[forwardOffsetDirectional] == 0x0000FF00) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x0000FFFF, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x0000FFFF, brightness, false);
          } else if (led_data->RGBdata[forwardOffsetDirectional] == 0x0000FFFF) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x000000FF, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x000000FF, brightness, false);
          } else if (led_data->RGBdata[forwardOffsetDirectional] == 0x000000FF) {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x00FF00FF, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FF00FF, brightness, false);
          } else {
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0x00FF0000, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FF0000, brightness, false);
          }
          return;
      } else if (led_mode == 8) { // Mullet
          led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional, 0xFFFFFFFF, brightness, true);
          if (led_data->RGBdata[rearOffsetDirectional] == 0x00FF0000) {
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FFFF00, brightness, false);
          } else if (led_data->RGBdata[rearOffsetDirectional] == 0x00FFFF00) {
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x0000FF00, brightness, false);
          } else if (led_data->RGBdata[rearOffsetDirectional] == 0x0000FF00) {
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x0000FFFF, brightness, false);
          } else if (led_data->RGBdata[rearOffsetDirectional] == 0x0000FFFF) {
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x000000FF, brightness, false);
          } else if (led_data->RGBdata[rearOffsetDirectional] == 0x000000FF) {
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FF00FF, brightness, false);
          } else {
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional, 0x00FF0000, brightness, false);
          }
          return;
      } else if (led_mode == 9){ // Knight Rider
          int centerPosition = (fmod(current_time, 1) * forwardlengthDirectional) + forwardOffsetDirectional;
          if(fmod(current_time, 2) > 1){
              centerPosition = forwardOffsetDirectional + forwardlengthDirectional - centerPosition + forwardOffsetDirectional;
          }
          for(int i = forwardOffsetDirectional; i < forwardOffsetDirectional + forwardlengthDirectional; i++){
              uint32_t intensity = (((forwardlengthDirectional/2)-min(abs(i-centerPosition), forwardlengthDirectional/2))
              /(forwardlengthDirectional/2.0)) * 255;
              led_set_color(led_data, i, intensity << 16, brightness, false);
          }
          centerPosition = (fmod(current_time, 1) * rearlengthDirectional) + rearOffsetDirectional;
          if(fmod(current_time, 2) > 1){
              centerPosition = rearOffsetDirectional + rearlengthDirectional - centerPosition + rearOffsetDirectional;
          }
          for(int i = rearOffsetDirectional; i < rearOffsetDirectional + rearlengthDirectional; i++){
              uint32_t intensity = (((rearlengthDirectional/2)-min(abs(i-centerPosition), rearlengthDirectional/2))/(rearlengthDirectional/2.0)) * 255;
              led_set_color(led_data, i, intensity << 16, brightness, false);
          }
          return;
      } else if (led_mode == 10){ // Felony
          int state = 0;
          if(led_data->RGBdata[forwardOffsetDirectional] == 0x00000000){
              state = 1;
          }else if(led_data->RGBdata[forwardOffsetDirectional] == 0x00FF0000){
              state = 2;
          }
          if(state == 0){
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional / 2, 0x00000000, brightness, false);
              led_strip_set_color(led_data, forwardOffsetDirectional + forwardlengthDirectional / 2, forwardlengthDirectional, 0x00FF0000, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional / 2, 0x00000000, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional + rearlengthDirectional / 2, rearlengthDirectional, 0x00FF0000, brightness, false);
          } else if (state == 1){
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional / 2, 0x00FF0000, brightness, false);
              led_strip_set_color(led_data, forwardOffsetDirectional + forwardlengthDirectional / 2, forwardlengthDirectional, 0x000000FF, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional / 2, 0x00FF0000, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional + rearlengthDirectional / 2, rearlengthDirectional, 0x000000FF, brightness, false);
          }else{
              led_strip_set_color(led_data, forwardOffsetDirectional, forwardlengthDirectional / 2, 0x000000FF, brightness, false);
              led_strip_set_color(led_data, forwardOffsetDirectional + forwardlengthDirectional / 2, forwardlengthDirectional, 0x00000000, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional, rearlengthDirectional / 2, 0x000000FF, brightness, false);
              led_strip_set_color(led_data, rearOffsetDirectional + rearlengthDirectional / 2, rearlengthDirectional, 0x00000000, brightness, false);
          }
          return;
      }

  }
  void led_update_after() {
    if(led_data->led_status_count > 0){
      statusPixels->show(); 
    }
    if(led_data->led_forward_count > 0){
      forwardPixels->show(); 
    }
    if(led_data->led_rear_count > 0){
      backwardPixels->show(); 
    }
  }
};
