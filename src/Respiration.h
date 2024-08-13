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
#include <numeric>

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
        float fromMinStdDev = -1;
        float fromMaxStdDev = 1;

    //-----TEMPERATURE SIGNAL-----//
        // Base temperature signal normalizer time window
        float normalizerTimeWindow = 5;

        // Base temperature signal smoothing factor
        float smootherFactor = 0.3;

    //-----EXHALE/INHALE-----//
    // Inhale trough ---> 0
    // Exhale peak ---> 1
        // Thresholds for peak detection (from normalized value)
        float normalizedPeakThreshold = 0;
        float normalizedTroughThreshold = 0;
        float normalizedPeakReloadThreshold = -0.1;
        float normalizedPeakFallbackThreshold = 0.1;
        float normalizedTroughReloadThreshold = 0.1;
        float normalizedTroughFallbackThreshold = 0.1;

        // Thresholds for peak detection (from MinMaxScaler scaled value)
        float minMaxScaledPeakThreshold = 0.5;
        float minMaxScaledTroughThreshold = 0.5;
        float minMaxScaledPeakReloadThreshold = 0.45;
        float minMaxScaledPeakFallbackThreshold = 0.1;
        float minMaxScaledTroughReloadThreshold = 0.55;
        float minMaxScaledTroughFallbackThreshold = 0.1;

    //-----AMPLITUDE-----//
        // Amplitude normalizer time window
        float amplitudeNormalizerTimeWindow = 90;

        // Amplitude variability normalizer time window
        float amplitudeVariabilityNormalizerTimeWindow = 30;

        // Amplitude smoothing factors
        float amplitudeSmootherFactor = 2;
        float amplitudeLevelSmootherFactor = 20;

    //-----RPM-----//
    // Respirations per minute
        // Rpm normalizer time window
        float rpmNormalizerTimeWindow = 90;

        // Rpm variability normalizer time window
        float rpmVariabilityNormalizerTimeWindow = 30;

        // Rpm smoothing factors
        float rpmSmootherFactor = 2;
        float rpmLevelSmootherFactor = 20;

    //-----PLAQUETTE OBJECTS-----//
        // Normalizers
        Normalizer normalizer;
        Normalizer amplitudeNormalizer;
        Normalizer amplitudeVariabilityNormalizer;
        Normalizer rpmNormalizer;
        Normalizer rpmVariabilityNormalizer;

        // Peak detectors
        PeakDetector normalizedPeak;
        PeakDetector normalizedTrough;
        // Peak detectors
        PeakDetector minMaxScaledPeak;
        PeakDetector minMaxScaledTrough;
      
        // Smoothers
        Smoother smoother;
        Smoother amplitudeSmoother;
        Smoother amplitudeLevelSmoother;
        Smoother rpmSmoother;
        Smoother rpmLevelSmoother;

        // MinMax Scaler
        MinMaxScaler minMaxScaler;
        MinMaxScaler amplitudeMinMaxScaler;
        MinMaxScaler rpmMinMaxScaler;
    
    //-----VARIABLES-----//
        // Temperature
        float _temperature;
        uint16_t _adcValue;
        float _clampScaled;
        float _minMaxScaled;

        // exhale (exhale = temperature peak)
        bool _normalizedExhale;
        bool _minMaxScaledExhale;

        // Amplitude
        float _amplitude;
        float _clampScaledAmplitude;
        float _minMaxScaledAmplitude;
        float _amplitudeLevel;
        float _amplitudeRoChange;
        float _amplitudeCV;

        // Rpm 
        uint8_t _interval;
        float _rpm;
        float _clampScaledRpm;
        float _minMaxScaledRpm;
        float _rpmLevel;
        float _rpmRoChange;
        float _rpmCV;

        float _min;
        float _max;

        static const int numberOfCycles = 5;
        int intervals[numberOfCycles] = {};
        int _millisPassed = 0;

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
  float getClampScaled(); //returns scaled base signal
  float getMinMaxScaled(); //returns scaled base signal
  bool normalizedIsExhaling() const; //returns true if user is exhaling (from normalized value)
  bool minMaxScaledIsExhaling() const; //returns true if user is exhaling (from MinMaxScaler scaled value)

  float getTemperatureAmplitude() const; //returns breah amplitude (temperature at peak - temperature at trough)
  float getNormalizedAmplitude(); //returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
  float getClampScaledAmplitude(); //returns scaled breath amplitude (0 to 1)
  float getMinMaxScaledAmplitude(); //returns scaled breath amplitude (0 to 1)
  float getAmplitudeLevel() const; //returns breath amplitude level indicator (0 : smaller than baseline, 0.5 : no significant change from baseline, 1 : larger than baseline)
  float getAmplitudeChange() const; //returns breath amplitude rate of change
  float getAmplitudeVariability() const; //returns breath amplitude coefficient of variation

  float getInterval() const; //returns interbreath interval
  float getRpm() const; //returns respiration rate (respirations per minute)
  float getNormalizedRpm(); //returns normalized respiration rate (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
  float getClampScaledRpm(); //returns scaled respiration rate (0 to 1)
  float getMinMaxScaledRpm(); //returns scaled respiration rate (0 to 1)
  float getRpmLevel() const; //returns repiration rate level indicator (0 : slower than baseline, 0.5 : no significant change from baseline, 1 : faster than baseline)
  float getRpmChange() const; //returns respiration rate rate of change
  float getRpmVariability() const; //returns respiration rate coefficient of variation 

  float getMin();
  float getMax();
};

#endif