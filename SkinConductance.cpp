/*
 * SkinConductance.cpp
 *
 * This class defines an object that can be used to gather information about
 * skin conductance (SC) -- also called galvanic skin respone (GSR) or electro-dermic
 * activity (EDA).
 *
 * This file is part of the BioData project
 * (c) 2018 Erin Gee
 *
 * Contributing authors:
 * (c) 2018 Erin Gee
 * (c) 2018 Sofian Audry
 * (c) 2017 Thomas Ouellet Fredericks
 * (c) 2017 Martin Peach
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SkinConductance.h"

SkinConductance::SkinConductance(uint8_t pin, unsigned long rate) :
  _pin(pin),
  gsrLop(0.0001), gsrHip(0.9999), gsrLop2(0.5), gsrLop3(0.001){
  setSampleRate(rate);
  reset();
}

void SkinConductance::reset() {
  gsrSensorReading = 0;
  gsrMinMax = MinMax();
  gsrLop = Lop(0.01);    //Lop(0.01) is good low pass for the signal
  gsrLop2 = Lop(0.03);
  gsrLop3 = Lop(0.005);
  gsrHip = Hip(0.5);
  
  gsrSensorFiltered = 0;
  gsrSensorLopFiltered = 0;
  gsrSensorAmplitude = 0;
  gsrSensorLopassed = 0;
  gsrSensorChangeFiltered = 0;

  prevSampleMicros = micros();

  // Perform one update.
  sample();
}

void SkinConductance::setSampleRate(unsigned long rate) {
  sampleRate = rate;
  microsBetweenSamples = 1000000UL / sampleRate;
}

void SkinConductance::update() {
  unsigned long t = micros();
  if (t - prevSampleMicros >= microsBetweenSamples) {
    // Perform updates.
    sample();
    prevSampleMicros = t;
  }
}

float SkinConductance::getSCR() const {
    return gsrSensorChange;
}

float SkinConductance::getSCL() const {
    return gsrSensorLopassed;
}

int SkinConductance::getRaw() const {
  return gsrSensorReading;
}

void SkinConductance::sample() {
    // Read sensor value and invert it.
    gsrSensorReading = 1023 - analogRead(_pin);
    
    // Smooth out the signal a bit
    gsrSensorLop = gsrLop.filter(gsrSensorReading);
    //gsrSensorLopassed = gsrLop3.filter(gsrSensorReading);
    gsrSensorLopassed = map(gsrLop3.filter(gsrSensorReading), 0,1023,0,1000) / 1000.0;
    // Min max the data
    gsrSensorFiltered = gsrMinMax.filter(gsrSensorLop);
    gsrSensorAmplitude = gsrMinMax.getMax() - gsrMinMax.getMin(); // keep track of the amplitude of the signal
    if (gsrSensorAmplitude > 8) gsrMinMax.reset();  // if the signal moves around by 8 ADC values reset minmax
    gsrSensorChange = gsrLop2.filter(gsrSensorFiltered);  // Smooth out the minmaxed data
    
}
