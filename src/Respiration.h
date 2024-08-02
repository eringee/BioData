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

    //-----TEMPERATURE SIGNAL-----//
        // Base temperature signal normalizer time window
        float normalizerTimeWindow = 10;

        // Base temperature signal smoothing factor
        float smootherFactor = 0.1;

        // Base temperature signal scaling time window
        float scalerTimeWindow = 10;

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
        float normalizerAmplitudeTimeWindow = 120;

        // Amplitude change normalizer time window
        float normalizerAmplitudeChangeTimeWindow = 60;

        // Amplitude variability normalizer time window
        float normalizerAmplitudeVariabilityTimeWindow = 30;

        // Amplitude smoothing factors
        float smootherAmplitudeFactor = 2;
        float smootherAmplitudeChangeFactor = 20;

        // Amplitude scaler time windows
        float scalerAmplitudeChangeTimeWindow = 90;

    //-----RPM-----//
    // Respirations per minute
        // Rpm normalizer time window
        float normalizerRpmTimeWindow = 120;

        // Rpm change normalizer time window
        float normalizerRpmChangeTimeWindow = 60;

        // Rpm variability normalizer time window
        float normalizerRpmVariabilityTimeWindow = 20;

        // Rpm smoothing factors
        float smootherRpmFactor = 2;
        float smootherRpmChangeFactor = 20;

        // Rpm scaler time windows
        float scalerRpmChangeTimeWindow = 90;

    //-----FLOW RATE-----//
    // Rate at which the breath signal fluctuates
        // Flow rate normalizer time window   
        float normalizerFlowRateTimeWindow = 0.01;

        // Flow rate smoother factors 
        float smootherFlowRateFactor = 0.1;

        // Flow rate delta timer
        float flowRateMetroTimer = 0.05;

        // Thresholds for peak detection
        float flowRatePeakThreshold = 0.5;
        float flowRatePeakReloadThreshold = 0.4;
        float flowRatePeakFallbackThreshold = 0.1;

    //-----PLAQUETTE OBJECTS-----//
        // Normalizers
        Normalizer normalizer;
        Normalizer normalizerAmplitude;
        Normalizer normalizerAmplitudeChange;
        Normalizer normalizerAmplitudeVariability;
        Normalizer normalizerRpm;
        Normalizer normalizerRpmChange;
        Normalizer normalizerRpmVariability;
        Normalizer normalizerFlowRate;

        // Peak detectors
        PeakDetector peak;
        PeakDetector trough;
        PeakDetector flowRatePeak;
      
        // Smoothers
        Smoother smoother;
        Smoother smootherAmplitude;
        Smoother smootherAmplitudeChange;
        Smoother smootherRpm;
        Smoother smootherRpmChange;
        Smoother smootherFlowRate;
        Smoother smootherFlowRateVariability;

        // Scalers
        MinMaxScaler scaler;
        MinMaxScaler scalerAmplitudeChange;
        MinMaxScaler scalerAmplitudeVariability;
        MinMaxScaler scalerRpmChange;
        MinMaxScaler scalerRpmVariability;

        // Metronome
        Metro flowRateMetro;
    
    //-----VARIABLES-----//
        // Temperature
        float _temperature;
        uint16_t _adcValue;

        // exhale (exhale = temperature peak)
        bool _exhale;

        // Amplitude
        float _amplitude;
        float _amplitudeChange;
        float _amplitudeDelta;
        float _amplitudeCV;

        // Rpm 
        uint8_t _interval;
        float _rpm;
        float _rpmChange;
        float _rpmDelta;
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
  void flowRate(float value); // flow rate data processing

  /// Returns raw ADC signal.
  uint16_t getRaw() const;

  /// Returns temperature signal in Celcius, converted with Steinhart-Hart equation.
  float getTemperature() const;

  /// Get normalized respiration signal.
  float getNormalized();

  float getScaled(); //returns scaled base signal
  bool isExhaling() const; //returns true if user is exhaling (temperature going up)
  float getTemperatureAmplitude() const; //returns breah amplitude (temperature at peak - temperature at trough)
  float getNormalizedAmplitude(); //returns normalized breath amplitude 

  ///Returns the average amplitude of signal mapped between 0.0 and 1.0.
  /* For example, if amplitude is average, returns 0.5,
   * if amplitude is below average, returns < 0.5
   * if amplitude is above average, returns > 0.5.
  */
  float getAmplitudeChange() const; //returns breath amplitude change indicator

  float getTemperatureAmplitudeDelta() const; //returns breath amplitude delta
  float getAmplitudeVariability() const; //returns breath amplitude coefficient of variation

  float getInterval() const; //returns interbreath interval
  float getRpm() const; //returns respiration rate (respirations per minute)
  float getNormalizedRpm(); //returns normalized respiration rate

    /// Returns the average bpm of signal mapped between 0.0 and 1.0.
  /* For example, if bpm is average, returns 0.5,
    * if bpm is below average, returns < 0.5
    * if bpm is above average, returns > 0.5.
  */
  float getRpmChange() const; //returns repiration rate change indicator
  float getRpmDelta() const; //returns respiration rate delta
  float getRpmVariability() const; //returns respiration rate coefficient of variation 
};

#endif