#ifndef _voltagetable
#define _voltagetable

#include <stdint.h>

struct VoltageTable{
//  const uint8_t volt_percent_Table[100] = {
//    89,
//    94,
//    99,
//    104,
//    109,
//    112,
//    114,
//    119,
//    121,
//    123,
//    124,
//    126,
//    128,
//    129,
//    132,
//    134,
//    137,
//    139,
//    144,
//    145,
//    146,
//    147,
//    148,
//    149,
//    149,
//    150,
//    151,
//    152,
//    153,
//    154,
//    156,
//    158,
//    159,
//    161,
//    162,
//    163,
//    164,
//    165,
//    166,
//    167,
//    168,
//    169,
//    169,
//    170,
//    171,
//    172,
//    173,
//    174,
//    175,
//    176,
//    177,
//    178,
//    179,
//    180,
//    181,
//    182,
//    182,
//    183,
//    184,
//    184,
//    185,
//    186,
//    187,
//    187,
//    188,
//    189,
//    189,
//    191,
//    192,
//    193,
//    194,
//    195,
//    196,
//    197,
//    198,
//    199,
//    200,
//    201,
//    202,
//    203,
//    204,
//    204,
//    205,
//    205,
//    206,
//    206,
//    207,
//    207,
//    208,
//    208,
//    209,
//    209,
//    210,
//    211,
//    212,
//    213,
//    214,
//    216,
//    218,
//    218,
//  };
  
//  float voltsToPercent100(float volts) {
//    int cellvoltage = volts * 100;
//    for (int i = 0; i < 100; i++) {
//      if (volt_percent_Table[i] + 200 >= cellvoltage) {
//        return i/100.0f;
//      }
//    }
//    return 1;
//  }

  float voltsToPercent100(float cellvoltage){
    int v = cellvoltage * 100;
    if(v < 89){
      return 0.0;
    }else if(v < 94){
      return 0.01;
    }else if(v < 99){
      return 0.02;
    }else if(v < 104){
      return 0.03;
    }else if(v < 109){
      return 0.04;
    }else if(v < 112){
      return 0.05;
    }else if(v < 114){
      return 0.06;
    }else if(v < 119){
      return 0.07;
    }else if(v < 121){
      return 0.08;
    }else if(v < 123){
      return 0.09;
    }else if(v < 124){
      return 0.10;
    }else if(v < 126){
      return 0.11;
    }else if(v < 128){
      return 0.12;
    }else if(v < 129){
      return 0.13;
    }else if(v < 132){
      return 0.14;
    }else if(v < 134){
      return 0.15;
    }else if(v < 137){
      return 0.16;
    }else if(v < 139){
      return 0.17;
    }else if(v < 144){
      return 0.18;
    }else if(v < 145){
      return 0.19;
    }else if(v < 146){
      return 0.20;
    }else if(v < 147){
      return 0.21;
    }else if(v < 148){
      return 0.22;
    }else if(v < 149){
      return 0.23;
    }else if(v < 149){
      return 0.24;
    }else if(v < 150){
      return 0.25;
    }else if(v < 151){
      return 0.26;
    }else if(v < 152){
      return 0.27;
    }else if(v < 153){
      return 0.28;
    }else if(v < 154){
      return 0.29;
    }else if(v < 156){
      return 0.30;
    }else if(v < 158){
      return 0.31;
    }else if(v < 159){
      return 0.32;
    }else if(v < 161){
      return 0.33;
    }else if(v < 162){
      return 0.34;
    }else if(v < 163){
      return 0.35;
    }else if(v < 164){
      return 0.36;
    }else if(v < 165){
      return 0.37;
    }else if(v < 166){
      return 0.38;
    }else if(v < 167){
      return 0.39;
    }else if(v < 168){
      return 0.4;
    }else if(v < 169){
      return 0.41;
    }else if(v < 169){
      return 0.42;
    }else if(v < 170){
      return 0.43;
    }else if(v < 171){
      return 0.44;
    }else if(v < 172){
      return 0.45;
    }else if(v < 173){
      return 0.46;
    }else if(v < 174){
      return 0.47;
    }else if(v < 175){
      return 0.48;
    }else if(v < 176){
      return 0.49;
    }else if(v < 177){
      return 0.50;
    }else if(v < 178){
      return 0.51;
    }else if(v < 179){
      return 0.52;
    }else if(v < 180){
      return 0.53;
    }else if(v < 181){
      return 0.54;
    }else if(v < 182){
      return 0.55;
    }else if(v < 182){
      return 0.56;
    }else if(v < 183){
      return 0.57;
    }else if(v < 184){
      return 0.58;
    }else if(v < 184){
      return 0.59;
    }else if(v < 185){
      return 0.6;
    }else if(v < 186){
      return 0.61;
    }else if(v < 187){
      return 0.62;
    }else if(v < 187){
      return 0.63;
    }else if(v < 188){
      return 0.64;
    }else if(v < 189){
      return 0.65;
    }else if(v < 189){
      return 0.66;
    }else if(v < 191){
      return 0.67;
    }else if(v < 192){
      return 0.68;
    }else if(v < 193){
      return 0.69;
    }else if(v < 194){
      return 0.7;
    }else if(v < 195){
      return 0.71;
    }else if(v < 196){
      return 0.72;
    }else if(v < 197){
      return 0.73;
    }else if(v < 198){
      return 0.74;
    }else if(v < 199){
      return 0.75;
    }else if(v < 200){
      return 0.76;
    }else if(v < 201){
      return 0.77;
    }else if(v < 202){
      return 0.78;
    }else if(v < 203){
      return 0.79;
    }else if(v < 204){
      return 0.8;
    }else if(v < 204){
      return 0.81;
    }else if(v < 205){
      return 0.82;
    }else if(v < 205){
      return 0.83;
    }else if(v < 206){
      return 0.84;
    }else if(v < 206){
      return 0.85;
    }else if(v < 207){
      return 0.86;
    }else if(v < 207){
      return 0.87;
    }else if(v < 208){
      return 0.88;
    }else if(v < 208){
      return 0.89;
    }else if(v < 209){
      return 0.9;
    }else if(v < 209){
      return 0.91;
    }else if(v < 210){
      return 0.92;
    }else if(v < 211){
      return 0.93;
    }else if(v < 212){
      return 0.94;
    }else if(v < 213){
      return 0.95;
    }else if(v < 214){
      return 0.96;
    }else if(v < 216){
      return 0.97;
    }else if(v < 218){
      return 0.98;
    }else if(v < 218){
      return 0.99;
    }else{
      return 1.0;
    }
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
