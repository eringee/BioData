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

#ifndef SKIN_CONDUCTANCE_H_
#define SKIN_CONDUCTANCE_H_

#include <PlaquetteLib.h>

namespace pq {

class SkinConductance : public Unit {

public:
  // Constructors.
  SkinConductance(Engine& engine = Engine::primary());   
  SkinConductance(unsigned long rate, Engine& engine = Engine::primary());   
  virtual ~SkinConductance() {}

  /// Resets all values.
  void reset();

  /// Sets sample rate.
  // void setSampleRate(unsigned long rate);

  virtual float get();
  virtual float put(float value);

  /// Returns Skin Conductance Response (SRC).
  float getSCR() const { return _scr; }

  /// Returns Skin Conductance Level (SCL).
  float getSCL() const { return _scl; }

  /// Returns raw signal as returned by analogRead() (inverted).
  float getRaw() const { return _value; }

  /// Returns normalized signal scaled in [0, 1].
  float getScaled() const { return _scaledValue; }

  float getFast() { return fastLowPass.get(); }

  // Performs the actual adjustments of signals and filterings.
  // Internal use: don't use directly, use update() instead.
  void sample();

protected:
  virtual void begin();
  virtual void step();

  Smoother slowLowPass;
  Smoother fastLowPass;

  Normalizer normalizer;
  MinMaxScaler minMaxScaler;

  Alarm calibrationTimer;

  // Raw value.
  float _value;

  float _scaledValue;

  float _scl;
  float _scr;

  // float gsrSensorFiltered;
  // float gsrSensorLopFiltered;
  // float gsrSensorChange;
  // float gsrSensorChangeFiltered;
  // float gsrSensorAmplitude;
  // float gsrSensorLop;
  // float gsrSensorLopassed;

  // Sample rate in Hz.
  unsigned long sampleRate;

  // // Internal use.
  // unsigned long microsBetweenSamples;
  // unsigned long prevSampleMicros;

};

}

#endif
