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

SkinConductance::SkinConductance(uint8_t pin) :
  _pin(pin),
  GSRaverager(1250),
  gsrLop(0.001) {
}

void SkinConductance::reset() {
  GSRaverager.clear();
  gsrSensorReading = 0;
  GSRmastered = 0;
  GSRav = 0;
  gsrMinMax = MinMax();
  gsrLop = Lop(0.001); //0.001
  gsrSensorFiltered = 0;
  gsrSensorAmplitude = 0;
  gsrSensorLop = 0;
}

void SkinConductance::update() {
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

float SkinConductance::getSCR() const {
  return GSRmastered;
}

float SkinConductance::getSCL() const {
  return GSRav;
}

int SkinConductance::getRaw() const {
  return gsrSensorReading;
}
