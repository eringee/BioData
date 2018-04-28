/*
 * Heart.cpp
 *
 * This class defines an object that can be used to gather information about
 * pulse as gathered through a photoplethysmograph such as the Pulse Sensor.
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
#include "Heart.h"

Heart::Heart(uint8_t pin, unsigned long rate) :
  _pin(pin),
  heartThresh(0.3, 0.6),              // if signal does not fall below (low, high) bounds than signal is ignored
  heartSensorAmplitudeLop(0.001),
  heartSensorBpmLop(0.001)
{
  setSampleRate(rate);
  reset();
}

void Heart::reset() {
  heartSensorAmplitudeLop = Lop(0.001);
  heartSensorBpmLop = Lop(0.001);
  heartSensorAmplitudeLopValueMinMax = MinMax();
  heartSensorBpmLopValueMinMax = MinMax();
  heartSensorReading = heartSensorFiltered = heartSensorAmplitude = 0;
  bpmChronoStart = millis();
  bpm = 60;
  beat = false;

  prevSampleMicros = micros();

  // Perform one update.
  sample();
}

void Heart::setSampleRate(unsigned long rate) {
  sampleRate = rate;
  microsBetweenSamples = 1000000UL / sampleRate;
}

void Heart::update() {
  unsigned long t = micros();
  if (t - prevSampleMicros >= microsBetweenSamples) {
    // Perform updates.
    sample();
    prevSampleMicros = t;
  }
}

float Heart::getNormalized() const {
  return heartSensorFiltered;
}

float Heart::amplitudeChange() const {
  return heartSensorAmplitudeLopValueMinMaxValue;
}

float Heart::bpmChange() const {
  return heartSensorBpmLopValueMinMaxValue;
}

bool Heart::beatDetected() const {
  return beat;
}

float Heart::getBPM() const {
  return bpm;
}

int Heart::getRaw() const {
  return heartSensorReading;
}

void Heart::sample() {
  // Read analog value if needed.
  heartSensorReading = analogRead(_pin);

  heartSensorFiltered = heartMinMax.filter(heartSensorReading);
  heartSensorAmplitude = heartMinMax.getMax() - heartMinMax.getMin();
  heartMinMax.adapt(0.01); // APPLY A LOW PASS ADAPTION FILTER TO THE MIN AND MAX

  heartSensorAmplitudeLopValue = heartSensorAmplitudeLop.filter(heartSensorAmplitude);
  heartSensorBpmLopValue =  heartSensorBpmLop.filter(bpm);

  heartSensorAmplitudeLopValueMinMaxValue = heartSensorAmplitudeLopValueMinMax.filter(heartSensorAmplitudeLopValue);
  heartSensorAmplitudeLopValueMinMax.adapt(0.001);
  heartSensorBpmLopValueMinMaxValue = heartSensorBpmLopValueMinMax.filter(heartSensorBpmLopValue);
  heartSensorBpmLopValueMinMax.adapt(0.001);

  beat = heartThresh.detect(heartSensorFiltered);

  if ( beat ) {
    unsigned long ms = millis();
    float temporaryBpm = 60000. / (ms - bpmChronoStart);
    bpmChronoStart = ms;
    if ( temporaryBpm > 30 && temporaryBpm < 200 ) // make sure the BPM is within bounds
      bpm = temporaryBpm;
  }
}
