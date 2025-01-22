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
 * (c) 2024 Luana Belinsky
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
#include <Arduino.h>
#include <Wire.h>  
#include <numeric>

#include "PlaquetteLib.h" //https://sofapirate.github.io/Plaquette/index.html

using namespace pq;

#ifndef RESP_H_
#define RESP_H_

class Respiration {

  public:   
  //==============CONSTRUCTORS==============//
  Respiration(unsigned long rate=50);   
  // Constructor 
  virtual ~Respiration() {};

  // Metro object for sample timing
  Metro sampleMetro;

   //-----COMMON PARAMETERS-----//
        // Normalizers have a target mean of 0 and standard deviation of 1
        float normalizerMean = 0; 
        float normalizerStdDev = 1;

        // When scaling normalized signal to 0-1, use these values as min and max to map
        float fromMinStdDev = -1;
        float fromMaxStdDev = 1;

    //-----TEMPERATURE SIGNAL-----//
        // Base temperature signal normalizer time window
        float normalizerTimeWindow = 5;

        // Base temperature signal smoothing factor
        float smootherFactor = 0.5;

    //-----PEAK DETECTION-----//
        // Thresholds for peak detection 
        float minMaxScaledPeakThreshold = 0.5;
        float minMaxScaledTroughThreshold = 0.5;
        float minMaxScaledPeakReloadThreshold = 0.45;
        float minMaxScaledPeakFallbackThreshold = 0.1;
        float minMaxScaledTroughReloadThreshold = 0.55;
        float minMaxScaledTroughFallbackThreshold = 0.1;

    //-----AMPLITUDE-----//
        // Amplitude normalizer time window
        float amplitudeNormalizerTimeWindow = 90;

        // Normalizer for amplitude variability
        float normalizerForAmplitudeVariabilityTimeWindow = 30;

        // Amplitude smoothing factors
        float amplitudeSmootherFactor = 2;
        float amplitudeLevelSmootherFactor = 5;
        float amplitudeRateOfChangeSmootherFactor = 2;

    //-----RPM-----//
    // Respirations per minute
        // Rpm normalizer time window
        float rpmNormalizerTimeWindow = 90;

        // Normalizer for rpm variability
        float normalizerForRpmVariabilityTimeWindow = 30;

        // Rpm smoothing factors
        float rpmSmootherFactor = 2;
        float rpmLevelSmootherFactor = 10;
        float rpmRateOfChangeSmootherFactor = 2;

    //-----PLAQUETTE OBJECTS-----//
        // Normalizers
        Normalizer normalizer;
        Normalizer amplitudeNormalizer;
        Normalizer normalizerForAmplitudeVariability;
        Normalizer rpmNormalizer;
        Normalizer normalizerForRpmVariability;

        // Peak detectors
        PeakDetector minMaxScaledPeak;
        PeakDetector minMaxScaledTrough;
      
        // Smoothers
        Smoother smoother;
        Smoother amplitudeSmoother;
        Smoother amplitudeLevelSmoother;
        Smoother amplitudeRateOfChangeSmoother;
        Smoother rpmSmoother;
        Smoother rpmLevelSmoother;
        Smoother rpmRateOfChangeSmoother;

        // MinMax Scaler
        MinMaxScaler minMaxScaler;
    
    //-----VARIABLES-----//
        // Sampling rate
        unsigned long _sampleRate;

        // Raw signal
        int _signal;
        float _minMaxScaled;

        // exhale (exhale = temperature peak)
        bool _exhale;

        // Amplitude
        float _amplitude;
        float _clampScaledAmplitude;
        float _amplitudeLevel;
        float _amplitudeRateOfChange;
        float _amplitudeCoefficientOfVariation;

        // Rpm 
        unsigned long _interval;
        float _rpm;
        float _clampScaledRpm;
        float _rpmLevel;
        float _rpmRateOfChange;
        float _rpmCoefficientOfVariation;

        static const int numberOfCycles = 5;
        int intervals[numberOfCycles] = {};
        int _millisPassed = 0;

//===========METHODS===========//
  // Initializes Plaquette objects and variables
  void initialize(unsigned long rate=50);

  // Sets sample rate.
  void setSampleRate(unsigned long rate=50);

  // Calls sample() at sampling rate
  void update(float signal = 0);

  void sample(float signal = 0);   // reads the signal and passes it to the signal processing functions
  void peakOrTrough(float value); // base signal processing and peak detection
  void amplitude(float value); // amplitude data processing
  void rpm(); // respiration rate data processing

  // Returns raw signal.
  unsigned long getRaw() const;

  float getNormalized() const; //returns normalized initial signal
  float getScaled() const; //returns min-max scaled initial signal
  bool isExhaling() const; //returns true if user is exhaling 

  unsigned long getRawAmplitude() const; //returns breah amplitude (difference bewteen maximum and minimum value in latest breath cycle)
  float getNormalizedAmplitude() const; //returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is lower than usual, +2 is higher than usual)
  float getScaledAmplitude() const; //returns scaled breath amplitude (float between 0 and 1) : scaled by mapping and clamping normalized amplitude
  float getAmplitudeLevel() const; //returns breath amplitude level indicator (float between 0 and 1) 
  //(latest amplitudes are generally ===> 0 :  smaller than baseline, 0.5 : similar to baseline, 1 : larger than baseline)
  float getAmplitudeChange() const; //returns breath amplitude rate of change
  float getAmplitudeVariability() const; //returns breath amplitude coefficient of variation

  unsigned long getInterval() const; //returns interbreath interval in milliseconds
  float getRpm() const; //returns respiration rate (respirations per minute)
  float getNormalizedRpm() const; //returns normalized respiration rate (target mean 0, stdDev 1) (example: -2 is lower than usual, +2 is higher than usual)
  float getScaledRpm() const; //returns scaled respiration rate (float between 0 and 1) : scaled by mapping and clamping normalized rpm
  float getRpmLevel() const; //returns repiration rate level indicator (float between 0 and 1)
  //(latest amplitudes are generally ===> 0 :  slower than baseline, 0.5 : similar to baseline, 1 : faster than baseline)
  float getRpmChange() const; //returns respiration rate rate of change
  float getRpmVariability() const; //returns respiration rate coefficient of variation 
};

#endif