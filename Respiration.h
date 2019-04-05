 /*
 * Respiration.h
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

#include "Average.h"
#include "MinMax.h"
#include "Threshold.h"
#include "Lop.h"

#ifndef RESP_H_
#define RESP_H_

class Respiration {

  // Analog pin the Respiration sensor is connected to.
  uint8_t _pin;

  unsigned long bpmChronoStart;

  MinMax respMinMax;
  Threshold respThresh;

  Lop respSensorAmplitudeLop;
  Lop respSensorBpmLop;

  float respSensorAmplitudeLopValue;

  float respSensorBpmLopValue;
  MinMax respSensorAmplitudeLopValueMinMax;

  float respSensorAmplitudeLopValueMinMaxValue;
  MinMax respSensorBpmLopValueMinMax;

  float respSensorBpmLopValueMinMaxValue;

  float respSensorFiltered;
  float respSensorAmplitude;

  float respSensorReading;

  float bpm;  // this value is fed to initialize your BPM before a breath is detected

  bool breath;// declares that we think a breath has occurred


  // Sample rate in Hz.
  unsigned long sampleRate;

  // Internal use.
  unsigned long microsBetweenSamples;
  unsigned long prevSampleMicros;

public:
  Respiration(uint8_t pin, unsigned long rate=50);   // default respiration samplerate is 50Hz
  virtual ~Respiration() {}

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

  /// Get normalized respiration signal.
  float getNormalized() const;

  /// Returns true if a breath was detected during the last call to update().
  bool breathDetected() const;

  /// Returns BPM (breaths per minute).
  float getBPM() const;

  /// Returns raw signal as returned by analogRead().
  int getRaw() const;

  ///Returns the average amplitude of signal mapped between 0.0 and 1.0.
  /* For example, if amplitude is average, returns 0.5,
   * if amplitude is below average, returns < 0.5
   * if amplitude is above average, returns > 0.5.
  */
  float amplitudeChange() const;

  /// Returns the average bpm of signal mapped between 0.0 and 1.0.
  /* For example, if bpm is average, returns 0.5,
    * if bpm is below average, returns < 0.5
    * if bpm is above average, returns > 0.5.
  */
  float bpmChange() const;

  // Performs the actual adjustments of signals and filterings.
  // Internal use: don't use directly, use update() instead.
  void sample();
};

#endif
