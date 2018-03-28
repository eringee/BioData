/*
 * SkinConductance.h
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

#include "Average.h"
#include "MinMax.h"
#include "Threshold.h"
#include "Lop.h"

#ifndef HEART_H_
#define HEART_H_

class Heart {

  // Analog pin the SC sensor is connected to.
  uint8_t _pin;

  unsigned long bpmChronoStart;

  MinMax heartMinMax;
  Threshold heartThresh;

  Lop heartSensorAmplitudeLop;
  Lop heartSensorBpmLop;

  float heartSensorAmplitudeLopValue;

  float heartSensorBpmLopValue;
  MinMax heartSensorAmplitudeLopValueMinMax;

  float heartSensorAmplitudeLopValueMinMaxValue;
  MinMax heartSensorBpmLopValueMinMax;

  float heartSensorBpmLopValueMinMaxValue;

  float heartSensorFiltered;
  float heartSensorAmplitude;

  float heartSensorReading;

  float bpm;  // this value is fed to initialize your BPM before a heartbeat is detected

  bool beat;

  // Sample rate in Hz.
  unsigned long sampleRate;

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
  // Internal use: don't use directly, use update() instead.
  void sample();
};

#endif
