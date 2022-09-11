/*
 * Respiration.cpp
 *
 * This class defines an object that can be used to gather information about
 * respiration. There are many ways to gather respiration data, such as through thermistor,
 * thermopile, conductive rubber cord, or piezoelectric signals.
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
#include "Respiration.h"

Respiration::Respiration(uint8_t pin, unsigned long rate) :
  _pin(pin),

// look for center of min max signal - false triggers from noise are unlikely
respThresh(0.5, 0.55),          // if signal does not fall below (low, high) bounds than signal is ignored

respSensorAmplitudeLop(0.001),  // original value 0.001
  respSensorBpmLop(0.001)       // original value 0.001
{
  setSampleRate(rate);
  reset();
}

void Respiration::reset() {
  respSensorAmplitudeLop = Lop(0.001);  // original value 0.001
  respSensorBpmLop = Lop(0.001);        // original value 0.001
  respSensorAmplitudeLopValueMinMax = MinMax();
  respSensorBpmLopValueMinMax = MinMax();
  respSensorReading = respSensorFiltered = respSensorAmplitude = 0;
  bpmChronoStart = millis();   // Note that in this case, BPM refers to "breaths per minute"  ;)
  bpm = 12;
  breath = false;

  prevSampleMicros = micros();

  // Perform one update.
  sample();
}

void Respiration::setSampleRate(unsigned long rate) {
  sampleRate = rate;
  microsBetweenSamples = 1000000UL / sampleRate;  //
}

void Respiration::update() {
  unsigned long t = micros();
  if (t - prevSampleMicros >= microsBetweenSamples) {
    // Perform updates.
    sample();
    prevSampleMicros = t;
  }
}

float Respiration::getNormalized() const {
  return respSensorFiltered;
}

float Respiration::amplitudeChange() const {
  return respSensorAmplitudeLopValueMinMaxValue;
}

float Respiration::bpmChange() const {
  return respSensorBpmLopValueMinMaxValue;
}

bool Respiration::breathDetected() const {
  return breath;
}

float Respiration::getBPM() const {
  return bpm;
}

int Respiration::getRaw() const {
  return respSensorReading;
}

void Respiration::sample() {
  // Read analog value if needed.
  respSensorReading = analogRead(_pin); //this is a dummy read to clear the adc.  This is needed at higher sampling frequencies.
  respSensorReading = analogRead(_pin);
  
  respSensorFiltered = respMinMax.filter(respSensorReading);
  respSensorAmplitude = respMinMax.getMax() - respMinMax.getMin();
  respMinMax.adapt(0.05); // APPLY A LOW PASS ADAPTION FILTER TO THE MIN AND MAX

  respSensorAmplitudeLopValue = respSensorAmplitudeLop.filter(respSensorAmplitude);
  respSensorBpmLopValue =  respSensorBpmLop.filter(bpm);

  respSensorAmplitudeLopValueMinMaxValue = respSensorAmplitudeLopValueMinMax.filter(respSensorAmplitudeLopValue);
  respSensorAmplitudeLopValueMinMax.adapt(0.001);// original value 0.001
  respSensorBpmLopValueMinMaxValue = respSensorBpmLopValueMinMax.filter(respSensorBpmLopValue);
  respSensorBpmLopValueMinMax.adapt(0.001); // original value 0.001


  breath = respThresh.detect(respSensorFiltered);

  if ( breath ) {
    unsigned long ms = millis();
    float temporaryBpm = 60000. / (ms - bpmChronoStart);  // divide by 60 seconds
    bpmChronoStart = ms;
    if ( temporaryBpm > 3 && temporaryBpm < 60 ) { // make sure the BPM is within bounds
      bpm = temporaryBpm;
    }
  }
}
