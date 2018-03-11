/*
 * Heart.h
 *
 * This class defines an object that can be used to gather information from a photoplethysmograph
 *  -- also known as pulse sensor.
 *
 * This file is part of the BioData project
 * (c) 2018 Erin Gee   http://www.eringee.net
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

#include "Average.h"
#include "MinMax.h"
#include "Threshold.h"
#include "Lop.h"

#ifndef HEART_H_
#define HEART_H_

class Heart {

  // Analog pin the pulse sensor is connected to.
  uint8_t _pin;
  
  // used to start counting
  unsigned long bpmChronoStart; //milliseconds
 
  // Sample rate in Hz.
  unsigned long sampleRate;
    
  // changes to TRUE(1) when a beat occurs, otherwise reads as FALSE(0)
  bool beat;

  // threshold to determine if a heartbeat has occurred
  Threshold heartThresh;
    
  // raw ADC value of reading
  float heartSensorReading;
  
  // normalize the raw reading using its minimum and maximum values
  MinMax heartMinMax;
  float heartSensorFiltered; // raw value normalized from 0.0 to 1.0

  // raw amplitude of ADC values
  float heartSensorAmplitude;
  
  // apply low-pass filter to the amplitude of the raw signal and output deviation from average amplitude
  Lop heartSensorAmplitudeLop;
  float heartSensorAmplitudeLopValue; // low-passed value of amplitude of raw signal
  MinMax heartSensorAmplitudeLopValueMinMax;
  float heartSensorAmplitudeLopValueMinMaxValue; // amplitude ranged from 0.0 to 1.0, 0.5 being average
   
  // apply low-pass filter to the BPM and output deviation from average BPM
  Lop heartSensorBpmLop;
  float heartSensorBpmLopValue; // low-passed value of BPM
  MinMax heartSensorBpmLopValueMinMax;
  float heartSensorBpmLopValueMinMaxValue; // BPM average ranged from 0.0 to 1.0, 0.5 being average

  // Internal use.
  unsigned long microsBetweenSamples;
  unsigned long prevSampleMicros;

public:
  Heart(uint8_t pin, unsigned long rate=500);
  virtual ~Heart() {}

  /// Resets all values.
  void reset();

  /// Sets sample rate.
  void setSampleRate(unsigned long rate);

  /**
   * Reads the signal and perform filtering operations. Call this before
   * calling any of the access functions. This function takes into account
   * the sample rate.
   */
  void update();

  /// Get normalized heartrate signal.
  float getNormalized() const;

  /// Returns true if a beat was detected during the last call to update().
  bool beatDetected() const;

  /// Returns BPM (beats per minute).
  float getBPM() const;

  /// Returns raw signal as returned by analogRead().
  int getRaw() const;

  // Performs the actual adjustments of signals and filterings.
  void sample();
};

#endif
