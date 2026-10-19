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

SkinConductance::SkinConductance(unsigned long rate)
{
  initialize(rate);
}

void SkinConductance::initialize(
    unsigned long rate, std::optional<unsigned long> initialMicros)
{
  gsrSensorSignal = 0;

  gsrSensorFiltered = 0;
  gsrSensorLopFiltered = 0;
  gsrSensorAmplitude = 0;
  gsrSensorLopassed = 0;
  gsrSensorChangeFiltered = 0;

  timer.start();

  prevSampleMicros = initialMicros ? *initialMicros : timer.getMicros();
  setSampleRate(rate);
}

void SkinConductance::setSampleRate(unsigned long rate) {
  _sampleRate = rate;
  microsBetweenSamples = 1000000UL / _sampleRate;
}

void SkinConductance::update(float signal, unsigned long elapsedMicros) {
  if(elapsedMicros - prevSampleMicros >= microsBetweenSamples)
  {
    sample(signal);
    prevSampleMicros = elapsedMicros;
  }
}

void SkinConductance::update(float signal) {
  update(signal, timer.getMicros());
}

float SkinConductance::getSCR() const {
    return gsrSensorChange;
}

float SkinConductance::getSCL() const {
    return gsrSensorLopFiltered;
}

int32_t SkinConductance::getRaw() const {
  return gsrSensorSignal;
}

void SkinConductance::sample(float signal) {
    // Read sensor value and invert it.
    // TODO : How do we do this if the signal comes from a 16-bit ADC or a measurement? Normalize or scale before inverting?
    gsrSensorSignal = signal; 
    float gsrSensorInverted = 1023 - signal; 
    
    gsrSensorLop = alpha_1*gsrSensorInverted + (1 - alpha_1)*gsrSensorLop;
    gsrSensorLopassed = alpha_2*gsrSensorLop + (1 - alpha_2)*gsrSensorLopassed;

    gsrSensorChange = ((gsrSensorLop - gsrSensorLopassed)/10)+0.2;

    gsrSensorLopFiltered = mapper(gsrSensorLop, 1023, 0, 0, 1000)*0.001;

    gsrSensorChange = clamp(gsrSensorChange, 0, 1);
    // TODO : is the raw signal we want to give out inverted or not? option to get raw input and inverted signal??
}

