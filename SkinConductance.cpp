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
  GSRaverager(1250),
  gsrLop(0.001) {
    setSampleRate(rate);
    reset();
}

void SkinConductance::reset() {
  GSRaverager.clear();
  gsrSensorReading = 0;
  GSRmastered = 0;
  GSRav = 0;
  gsrMinMax = MinMax();
  gsrLop = Lop(0.001);
  gsrSensorFiltered = 0;
  gsrSensorAmplitude = 0;
  gsrSensorLop = 0;

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
  return GSRmastered;
}

float SkinConductance::getSCL() const {
  return GSRav;
}

int SkinConductance::getRaw() const {
  return gsrSensorReading;
}

void SkinConductance::sample() {
  // Read sensor value and invert it.
  gsrSensorReading = 1023 - analogRead(_pin);

  // Apply filters to the signal.
  gsrSensorLop = gsrLop.filter(gsrSensorReading);
  gsrSensorFiltered = gsrMinMax.filter(gsrSensorLop); // Min max the data
  gsrMinMax.adapt(0.01);   // APPLY A LOW PASS ADAPTION FILTER TO THE MIN AND MAX

  // Compute an average GSR over time.
  // Only the greater than average values matter, shove the lesser than values down quick.
  if(gsrSensorLop<0) GSRaverager.push(gsrSensorLop-10); // value too low, shove it down
  else GSRaverager.push(gsrSensorLop);

  // Compute mastered signal.
  GSRav = GSRaverager.mean();
  GSRmastered = (gsrSensorLop-GSRav) / 10;
  // if(gsrSensorLop>GSRmax) GSRmax = gsrSensorLop;      //keep track of the latest max value
  // else GSRmax = (GSRaverager.maximum(&maxat));

  // Keep the mapping within 0..1 range
  GSRmastered = constrain(GSRmastered, 0, 1);
}
