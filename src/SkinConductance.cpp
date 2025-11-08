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

namespace pq {

SkinConductance::SkinConductance(Engine& engine) : SkinConductance(50, engine) {}
SkinConductance::SkinConductance(unsigned long rate, Engine& engine) :
  Unit(engine),
//  sampleMetro(rate, engine),
  slowLowPass(12.0, engine),
  fastLowPass(1.0, engine),
  normalizer(engine),
  minMaxScaler(engine)
{
//  setSampleRate(rate);
  normalizer.timeWindow(minutesToSeconds(10));
  minMaxScaler.timeWindow(minutesToSeconds(10));

  reset();
}


float SkinConductance::get() {
  return getScaled();
}

float SkinConductance::put(float value) {
  _value = value;
  return get();
}

void SkinConductance::reset() {
}

// void SkinConductance::setSampleRate(unsigned long rate) {
//   sampleRate = rate;
//   microsBetweenSamples = 1000000UL / sampleRate;
// }

void SkinConductance::sample() {
  // Rescale value.
  _value >> normalizer >> minMaxScaler;

  // Compute low-pass filters (slow and fast).
  minMaxScaler >> slowLowPass;
  minMaxScaler >> fastLowPass;

  // Compute values for SCL and SCR.
  _scl = slowLowPass;
  _scr = max(fastLowPass - slowLowPass, 0);

  _scaledValue = minMaxScaler;
}

void SkinConductance::begin() {
  reset();
}

void SkinConductance::step() {
  sample();
}

}
