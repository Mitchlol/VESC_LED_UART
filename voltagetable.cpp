#ifndef _voltagetable
#define _voltagetable

#include <stdint.h>

struct VoltageTable{
  const uint8_t volt_percent_Table[100] = {
    89,
    94,
    99,
    104,
    109,
    112,
    114,
    119,
    121,
    123,
    124,
    126,
    128,
    129,
    132,
    134,
    137,
    139,
    144,
    145,
    146,
    147,
    148,
    149,
    149,
    150,
    151,
    152,
    153,
    154,
    156,
    158,
    159,
    161,
    162,
    163,
    164,
    165,
    166,
    167,
    168,
    169,
    169,
    170,
    171,
    172,
    173,
    174,
    175,
    176,
    177,
    178,
    179,
    180,
    181,
    182,
    182,
    183,
    184,
    184,
    185,
    186,
    187,
    187,
    188,
    189,
    189,
    191,
    192,
    193,
    194,
    195,
    196,
    197,
    198,
    199,
    200,
    201,
    202,
    203,
    204,
    204,
    205,
    205,
    206,
    206,
    207,
    207,
    208,
    208,
    209,
    209,
    210,
    211,
    212,
    213,
    214,
    216,
    218,
    218,
  };
  
  float voltsToPercent100(float volts) {
    int cellvoltage = volts * 100;
    for (int i = 0; i < 100; i++) {
      if (volt_percent_Table[i] + 200 >= cellvoltage) {
        return i/100.0f;
      }
    }
    return 1;
  }
  
  float voltsToPercent5(float volts) {
     if(volts > 4.1){
      return 1.0f;
     }else if(volts > 3.95){
      return 0.8f;
     }else if(volts > 3.8){
      return 0.6f;
     }else if(volts > 3.6){
      return 0.4f;
     }else if(volts > 3.33){
      return 0.2f;
     }else{
      return 0.0f;
     }
  }
};

#endif
