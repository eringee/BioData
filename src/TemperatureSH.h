//
//    FILE: TemperatureSH.h
//  AUTHOR: Luana Belinsky (adapted from SHThermistor from citriena)
// VERSION: 0.1.0
//    DATE: 2023-07-09
// PURPOSE: Library for Steinhart-Hart temperature calculation for thermistor
//     URL: **
//
// MIT License

// Copyright (c) 2021 citriena

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef TEMPERATURESH_h
#define TEMPERATURESH_h
#include <Arduino.h> 

//ADC Resolution
#define ADC_10BIT_VALUE  1023 // 0x00400
#define ADC_12BIT_VALUE  4095 // 0x01000
#define ADC_14BIT_VALUE 16383 // 0x04000
#define ADC_16BIT_VALUE 65535 // 0x10000
//ADS1115 Resolution used as default value
#define DEFAULT_EXCITE_VALUE ADC_16BIT_VALUE

//Default voltage input
#define DEFAULT_VOLTAGE_IN 3.3 // 3.3V default voltage input

//ADC Gain
// Adjustment factor to convert ADC value to voltage depending on ADC gain
// needed for use with external ADC ADS1115
#define ADC_GAIN_6_144V  6.144 * 2
#define ADC_GAIN_4_096V  4.096 * 2
#define ADC_GAIN_2_048V  2.048 * 2
#define ADC_GAIN_1_024V  1.024 * 2
#define ADC_GAIN_0_512V  0.512 * 2
#define ADC_GAIN_0_256V  0.256 * 2
// 4.096V gain used with ADS1115 to maximize range with 3.3V
#define DEFAULT_ADC_GAIN ADC_GAIN_4_096V 

//Data from MA100 Amphenol Thermometrics NTC Thermistor used as default values
// see datasheet for specifications
#define DEFAULT_DIV_R 10000 // 10k default divider resistor value
#define DEFAULT_SH_T1 15.0 // 15C default low temperature reference
#define DEFAULT_SH_T2 30.0 // 30C default mid temperature reference
#define DEFAULT_SH_T3 45.0 // 45C default high temperature reference
#define DEFAULT_SH_R1 16031 // default low thermistor resistance
#define DEFAULT_SH_R2 7869 // default mid thermistor resistance
#define DEFAULT_SH_R3 4267 // default high thermistor resistance

//Error
#define TH_ERR_DATA 1024  // ƒT[ƒ~ƒXƒ^’füŽž“™‚É•Ô‚·ƒf[ƒ^

typedef enum {
  NTC_EXCITE,
  NTC_GND,
} NTC_CONNECT_t;
#define DEFAULT_NTC_CONNECT NTC_EXCITE

class SHthermistor { 
public:
  SHthermistor(float SH_T1, float SH_T2, float SH_T3, // low, mid, and high temperature (Celcius) including actual measure condition
               float SH_R1, float SH_R2, float SH_R3, // thermistor resistance (ohm) at SH_T1, SH_T2, SH_T3
               float divR,                  // resistance value (ohm) of series divider resistor
               int16_t adcPin,              // analog pin connected to thermistor and series resistor
               NTC_CONNECT_t ntcConnect,    // connection of thermistor and series resistor
               float offsetT               // offset value added to calculated temperature
  );

  SHthermistor(int16_t adcPin);
  SHthermistor();

  void setSHcoef(float SH_T1, float SH_T2, float SH_T3, float SH_R1, float SH_R2, float SH_R3);
  
  void readResistance(int16_t ADC);
  void readResistance();

  float getResistance();
  float getTemperature();
  float r2temp(float r);
  float readTemp(int16_t ADC);
  float readTemp();
  void setDivR(float divR);

  void setOffsetTemp(float offsetT);
  float getSH_A();
  float getSH_B();
  float getSH_C();

protected:

  float SH_A;
  float SH_B;
  float SH_C;
  float _DIV_R;
  int16_t _ADC_CHANNEL;
  float _OFFSET_TEMP;
  NTC_CONNECT_t _NTC_CONNECT;
  int32_t _EXCITE_VALUE;
  float _V_IN;
  float _ADC_GAIN;
  int adcValue;
  float resistance;
  float temperature;
};

#endif