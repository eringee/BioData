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
 * (c) 2024 Luana Belinsky
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
#include <Arduino.h>
#include "ExternalADC.h"
#include "TemperatureSH.h"
#include <Wire.h>  

#include "PlaquetteLib.h" //https://sofapirate.github.io/Plaquette/index.html

using namespace pq;

#ifndef RESP_H_
#define RESP_H_

class Respiration {
  // Analog pin the Respiration sensor is connected to.
  uint8_t _pin;

  //ADS1115 object if using external ADC
  ADS1115 ADS;

  //SHthermistor object to calculate temperature from ADC value
  SHthermistor thermistor;

  // Sample rate in Hz.
  unsigned long sampleRate;

  // Internal use.
  unsigned long microsBetweenSamples;
  unsigned long prevSampleMicros;

public:

   //-----COMMON PARAMETERS-----//
        // Normalizers have a target mean of 0 and standard deviation of 1
        float normalizerMean = 0; 
        float normalizerStdDev = 1;

        // When scaling normalized signal to 0-1, use these values as min and max to map
        float fromMinStdDev = -3;
        float fromMaxStdDev = 3;

    //-----TEMPERATURE SIGNAL-----//
        // Base temperature signal normalizer time window
        float normalizerTimeWindow = 10;

        // Base temperature signal smoothing factor
        float smootherFactor = 0.1;

    //-----EXHALE/INHALE-----//
    // Inhale trough ---> 0
    // Exhale peak ---> 1
        // Thresholds for peak detection
        float peakThreshold = 0.5;
        float troughThreshold = 0.5;
        float peakReloadThreshold = 0.4;
        float peakFallbackThreshold = 0.05;
        float troughReloadThreshold = 0.6;
        float troughFallbackThreshold = 0.05;

    //-----AMPLITUDE-----//
        // Amplitude normalizer time window
        float normalizerAmplitudeTimeWindow = 90;

        // Amplitude variability normalizer time window
        float normalizerAmplitudeVariabilityTimeWindow = 30;

        // Amplitude smoothing factors
        float smootherAmplitudeFactor = 2;
        float smootherAmplitudeLevelFactor = 20;

    //-----RPM-----//
    // Respirations per minute
        // Rpm normalizer time window
        float normalizerRpmTimeWindow = 90;

        // Rpm variability normalizer time window
        float normalizerRpmVariabilityTimeWindow = 30;

        // Rpm smoothing factors
        float smootherRpmFactor = 2;
        float smootherRpmLevelFactor = 20;

    //-----PLAQUETTE OBJECTS-----//
        // Normalizers
        Normalizer normalizer;
        Normalizer normalizerAmplitude;
        Normalizer normalizerAmplitudeVariability;
        Normalizer normalizerRpm;
        Normalizer normalizerRpmVariability;

        // Peak detectors
        PeakDetector peak;
        PeakDetector trough;
      
        // Smoothers
        Smoother smoother;
        Smoother smootherAmplitude;
        Smoother smootherAmplitudeLevel;
        Smoother smootherRpm;
        Smoother smootherRpmLevel;
    
    //-----VARIABLES-----//
        // Temperature
        float _temperature;
        uint16_t _adcValue;
        float _scaled;

        // exhale (exhale = temperature peak)
        bool _exhale;

        // Amplitude
        float _amplitude;
        float _scaledAmplitude;
        float _amplitudeLevel;
        float _amplitudeChange;
        float _amplitudeCV;

        // Rpm 
        uint8_t _interval;
        float _rpm;
        float _scaledRpm;
        float _rpmLevel;
        float _rpmChange;
        float _rpmCV;

    //-----METHODS-----//
  Respiration(uint8_t pin, unsigned long rate=50);   // Constructor. Default respiration samplerate is 50Hz
  virtual ~Respiration() {}

  /// sets certain Plaquette object parameters
  void reset();

  /// Sets sample rate.
  void setSampleRate(unsigned long rate);

  /**
   * Reads the signal and perform filtering operations. Call this before
   * calling any of the access functions. This function takes into account
   * the sample rate.
   */
  void update();

  // Performs the actual adjustments of signals and filterings.
  // Internal use: don't use directly, use update() instead.
  void sample();

  void peakOrTrough(float value); // base temperature signal processing and peak detection
  void amplitude(float value); // amplitude data processing
  void rpm(); // respiration rate data processing

  /// Returns raw ADC signal.
  uint16_t getRaw() const;

  /// Returns temperature signal in Celcius, converted with Steinhart-Hart equation.
  float getTemperature() const;

  /// Get normalized respiration signal.
  float getNormalized();
  float getScaled(float min=0, float max=1); //returns scaled base signal
  bool isExhaling() const; //returns true if user is exhaling (temperature going up)

  float getTemperatureAmplitude() const; //returns breah amplitude (temperature at peak - temperature at trough)
  float getNormalizedAmplitude(); //returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
  float getScaledAmplitude(float min=0, float max=1); //returns scaled breath amplitude (default 0 to 1)
  float getAmplitudeLevel() const; //returns breath amplitude level indicator (0 : smaller than baseline, 0.5 : no significant change from baseline, 1 : larger than baseline)
  float getAmplitudeChange() const; //returns breath amplitude delta
  float getAmplitudeVariability() const; //returns breath amplitude coefficient of variation

  float getInterval() const; //returns interbreath interval
  float getRpm() const; //returns respiration rate (respirations per minute)
  float getNormalizedRpm(); //returns normalized respiration rate (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
  float getScaledRpm(float min=0, float max=1); //returns scaled respiration rate (default 0 to 1)
  float getRpmLevel() const; //returns repiration rate level indicator (0 : slower than baseline, 0.5 : no significant change from baseline, 1 : faster than baseline)
  float getRpmChange() const; //returns respiration rate delta
  float getRpmVariability() const; //returns respiration rate coefficient of variation 
};

#endif