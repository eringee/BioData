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
#include "TemperatureSH.h"
#include <Wire.h>  
#include <numeric>

#include "PlaquetteLib.h" //https://sofapirate.github.io/Plaquette/index.html

using namespace pq;

#ifndef RESP_H_
#define RESP_H_

typedef enum {
  _10_BITS = 1024,
  _12_BITS = 4096,
  _14_BITS = 16384,
  _16_BITS = 65536
} ADC_RESOLUTION;

class Respiration {
  // Analog pin the Respiration sensor is connected to.
  uint8_t _pin;

  //SHthermistor object to calculate temperature from ADC value
  SHthermistor thermistor;

  // Sample rate in Hz.
  unsigned long sampleRate;

  // Metro object for sample timing
  Metro sampleMetro;

public:

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

    //-----EXHALE/INHALE-----//
    // Inhale trough ---> 0
    // Exhale peak ---> 1
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
        // Temperature
        float _temperature;
        int _adcValue;
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

    //-----METHODS-----//
  Respiration(uint8_t pin, unsigned long rate=50, ADC_RESOLUTION resolution = _10_BITS);   // Constructor with pin + internal ADC. Default respiration samplerate is 50Hz
  Respiration(uint16_t (*getExternalADCValue)(), unsigned long rate=50, ADC_RESOLUTION resolution = _16_BITS);   // Constructor with external ADC. Default respiration samplerate is 50Hz
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
  float getScaled(); //returns minMax scaled base signal
  bool isExhaling() const; //returns true if user is exhaling 

  float getTemperatureAmplitude() const; //returns breah amplitude (temperature at peak - temperature at trough)
  float getNormalizedAmplitude(); //returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
  float getScaledAmplitude(); //returns scaled breath amplitude (0 to 1)
  float getAmplitudeLevel() const; //returns breath amplitude level indicator (0 : smaller than baseline, 0.5 : no significant change from baseline, 1 : larger than baseline)
  float getAmplitudeChange() const; //returns breath amplitude rate of change
  float getAmplitudeVariability() const; //returns breath amplitude coefficient of variation

  unsigned long getInterval() const; //returns interbreath interval
  float getRpm() const; //returns respiration rate (respirations per minute)
  float getNormalizedRpm(); //returns normalized respiration rate (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
  float getScaledRpm(); //returns scaled respiration rate (0 to 1)
  float getRpmLevel() const; //returns repiration rate level indicator (0 : slower than baseline, 0.5 : no significant change from baseline, 1 : faster than baseline)
  float getRpmChange() const; //returns respiration rate rate of change
  float getRpmVariability() const; //returns respiration rate coefficient of variation 

private: 
 uint16_t (*_getExternalADCValue)(void);
};

#endif