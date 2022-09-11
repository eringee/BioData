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


float alpha_1 = 0.01;
float alpha_2 = 0.005;

SkinConductance::SkinConductance(uint8_t pin, unsigned long rate) :
  _pin(pin)
{
  setSampleRate(rate);
  reset();
}

void SkinConductance::reset() {
  gsrSensorReading = 0;

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
    return gsrSensorLopFiltered;
}

int SkinConductance::getRaw() const {
  return gsrSensorReading;
}

void SkinConductance::sample() {
    // Read sensor value and invert it.
    gsrSensorReading = 1023 - analogRead(_pin); //this is a dummy read to clear the adc.  This is needed at higher sampling frequencies.
    gsrSensorReading = 1023 - analogRead(_pin);
    // Smooth out the signals that you compare to one another and map between 0 and 1000

    gsrSensorLop = alpha_1*gsrSensorReading + (1 - alpha_1)*gsrSensorLop;
    gsrSensorLopassed = alpha_2*gsrSensorLop + (1 - alpha_2)*gsrSensorLopassed;

    gsrSensorChange = ((gsrSensorLop - gsrSensorLopassed)/10)+0.2;

    gsrSensorLopFiltered = map(gsrSensorLop, 0, 1023, 0, 1000)*0.001;

    gsrSensorChange = constrain(gsrSensorChange, 0, 1);

}
