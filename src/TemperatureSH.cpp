//
//    FILE: TemperatureSH.cpp
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

#include "TemperatureSH.h"

// STEINHART & HART EQUATION: 1/T = a + b(lnR) + c(lnR)^3
// Solve three simultaneous equations to obtain coefficients a,b,c:
// set thermistor resistance SH_R1, SH_R2, SH_R3 (ohm) at SH_T1, SH_T2, SH_T3 (Celcius)
// John M. Zurbuchen. Precision thermistor thermometry. Measurement Science Conference Tutorial: Thermometry-Fundamentals and Practice, 2000.
// http://www.nktherm.com/tec/linearize.html  Steinhart and Hart 式によるサーミスタ抵抗値の温度変換

//Constructor that takes reference temperatures and resistance values, resistance divider, type of connection, offset and ADC resolution
SHthermistor::SHthermistor(float SH_T1, float SH_T2, float SH_T3, float SH_R1, float SH_R2, float SH_R3, float divR, NTC_CONNECT_t ntcConnect, float offsetT, int resolution) :
  _DIV_R(divR),
  _OFFSET_TEMP(offsetT),
  _NTC_CONNECT(ntcConnect),
  _EXCITE_VALUE(resolution),
  _V_IN(DEFAULT_VOLTAGE_IN),
  _ADC_GAIN(DEFAULT_ADC_GAIN),
  adcValue(0),
  resistance(0),
  temperature(0)
{
  setSHcoef(SH_T1, SH_T2, SH_T3, SH_R1, SH_R2, SH_R3);
}

//Constructor that takes the resolution of the ADC as an argument
SHthermistor::SHthermistor(int resolution) :
  _DIV_R(DEFAULT_DIV_R),
  _OFFSET_TEMP(0),
  _NTC_CONNECT(DEFAULT_NTC_CONNECT),
  _EXCITE_VALUE(resolution),
  _V_IN(DEFAULT_VOLTAGE_IN),
  _ADC_GAIN(DEFAULT_ADC_GAIN),
  adcValue(0),
  resistance(0),
  temperature(0)
{
  setSHcoef(DEFAULT_SH_T1, DEFAULT_SH_T2, DEFAULT_SH_T3, DEFAULT_SH_R1, DEFAULT_SH_R2, DEFAULT_SH_R3);
}


//Set Steinhart-Hart coefficients
void SHthermistor::setSHcoef(float SH_T1, float SH_T2, float SH_T3, float SH_R1, float SH_R2, float SH_R3) {
  SH_T1 += 273.15;
  SH_T2 += 273.15;
  SH_T3 += 273.15;
  float SH_X1 = log(SH_R1);
  float SH_X2 = log(SH_R2);
  float SH_X3 = log(SH_R3);

  float SH_S = (SH_X1 - SH_X2);
  float SH_U = (SH_X1 - SH_X3);
  float SH_V = (1 / SH_T1 - 1 / SH_T2);
  float SH_W = (1 / SH_T1 - 1 / SH_T3);

  SH_C = (SH_V - SH_S * SH_W / SH_U) / ((pow(SH_X1, 3) - pow(SH_X2, 3)) - SH_S * ( pow(SH_X1, 3) - pow(SH_X3, 3)) / SH_U);  // Coefficient c
  SH_B = (SH_V - SH_C * (pow(SH_X1, 3) - pow(SH_X2, 3))) / SH_S;                                                            // Coefficient b
  SH_A = 1 / SH_T1 - SH_C * pow(SH_X1, 3) - SH_B * SH_X1;                                                                   // Coefficient a
}

//Reads resistance with ADC value input
void SHthermistor::readResistance(int ADC) {
    adcValue = float(ADC);

    float voltageOut = (adcValue / float(_EXCITE_VALUE)) * _ADC_GAIN;

  if (_NTC_CONNECT == NTC_GND) {
    resistance = (voltageOut * _DIV_R)/(_V_IN-voltageOut);
   } else {
    resistance = ((_V_IN * _DIV_R)/voltageOut)-_DIV_R;
   }  
 }

//Returns resistance
float SHthermistor::getResistance(){
  return resistance;
}

//Returns temperature
float SHthermistor::getTemperature(){
  return temperature;
}

//Converts resistance to temperature
float SHthermistor::r2temp(float resistance) { // calculate temperature from thermistor resistance using Steinhart-Hart equation
  if (resistance == 0) return TH_ERR_DATA;
  return (1 / (SH_A + SH_B * log(resistance) + SH_C * pow(log(resistance), 3)) - 273.15 + _OFFSET_TEMP); // return temperature in Celcius
}

//Reads temperature with ADC value input
float SHthermistor::readTemp(int ADC) {
  readResistance(ADC);
  temperature = r2temp(getResistance());
  return temperature;
}

//Sets resistance divider
void SHthermistor::setDivR(float divR) {
  _DIV_R = divR;
}

//Sets offset temperature
void SHthermistor::setOffsetTemp(float offsetTemp) {
  _OFFSET_TEMP = offsetTemp;
}

//Returns Steinhart-Hart coefficient a
float SHthermistor::getSH_A() {
  return SH_A;
}

//Returns Steinhart-Hart coefficient b
float SHthermistor::getSH_B() {
  return SH_B;
}

//Returns Steinhart-Hart coefficient c
float SHthermistor::getSH_C() {
  return SH_C;
}


