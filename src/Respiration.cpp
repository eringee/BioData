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

namespace pq {

//=============================================CONSTRUCTORS=============================================//
// constructor when using internal ADC 


Respiration::Respiration(Engine& engine) : Respiration(50, engine) {}
Respiration::Respiration(unsigned long rate, Engine& engine) :
  // Sub-units.
  normalizer(normalizerMean, normalizerStdDev, normalizerTimeWindow, engine), 
  amplitudeNormalizer(normalizerMean, normalizerStdDev, amplitudeNormalizerTimeWindow, engine),
  normalizerForAmplitudeVariability(normalizerMean, normalizerStdDev, normalizerForAmplitudeVariabilityTimeWindow, engine),
  rpmNormalizer(normalizerMean, normalizerStdDev, rpmNormalizerTimeWindow, engine),
  normalizerForRpmVariability(normalizerMean, normalizerStdDev, normalizerForRpmVariabilityTimeWindow, engine),
  minMaxScaledPeak(minMaxScaledPeakThreshold, PEAK_MAX, engine),
  minMaxScaledTrough(minMaxScaledTroughThreshold, PEAK_MIN, engine),
  smoother(smootherFactor, engine),
  amplitudeSmoother(amplitudeSmootherFactor, engine),
  amplitudeLevelSmoother(amplitudeLevelSmootherFactor, engine),
  amplitudeRateOfChangeSmoother(amplitudeRateOfChangeSmootherFactor, engine),
  rpmSmoother(rpmSmootherFactor, engine),
  rpmLevelSmoother(rpmLevelSmootherFactor, engine),
  rpmRateOfChangeSmoother(rpmRateOfChangeSmootherFactor, engine),
  minMaxScaler(engine),
  calibrationTimer(normalizerTimeWindow, engine),
  // Variables.
  _value(0),
  _minMaxScaled(0.5),
  _exhale(0),
  _amplitude(-FLT_MIN),
  _clampScaledAmplitude(0.5),
  _amplitudeLevel(0.5),
  _amplitudeRateOfChange(0),
  _amplitudeCoefficientOfVariation(0),
  _interval(0),
  _rpm(12),
  _clampScaledRpm(0.5),
  _rpmLevel(0.5),
  _rpmRateOfChange(0),
  _rpmCoefficientOfVariation(0),
  _millisPassed(0)
{
  setSampleRate(rate);
}

float Respiration::get() {
  return getNormalized();
}

float Respiration::put(float value) {
  _value = value;
  return get();
}

//=================================================SET=============================================//
// Sets certain Plaquette object parameters
void Respiration::reset() {
  //set peak detector thresholds
  minMaxScaledPeak.reloadThreshold(minMaxScaledPeakReloadThreshold);
  minMaxScaledPeak.fallbackTolerance(minMaxScaledPeakFallbackThreshold);
  minMaxScaledTrough.reloadThreshold(minMaxScaledTroughReloadThreshold);
  minMaxScaledTrough.fallbackTolerance(minMaxScaledTroughFallbackThreshold);

  //set scaler time window (slower than normalizer)
  minMaxScaler.timeWindow(20 * normalizerTimeWindow);

  normalizer.noClamp();

  calibrationTimer.start();

  // Perform one update.
  sample();
}

// Sets sample rate
void Respiration::setSampleRate(unsigned long rate) {
  sampleMetro.frequency(rate);
}

//=============================================UPDATE=============================================//


//==============================================GET=================================================//
// Returns raw ADC value
float Respiration::getRaw()  const {
  return _value ;
}

//Returns normalized ADCsignal (target mean 0, stdDev 1) (example: -2 is lower than usual, +2 is higher than usual)
float Respiration::getNormalized() const {  
  return normalizer;
} 

//Returns scaled ADC signal (float between 0 and 1) : scaled by minMaxScaler
float Respiration::getScaled() const { 
  return _minMaxScaled;
}

//Returns true if user is exhaling 
bool Respiration::isExhaling() const{ 
  return _exhale;
}

//Returns raw breah amplitude (ADC value)
unsigned long Respiration::getRawAmplitude() const{ 
  return max(_amplitude, 0);
}

 //Returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is lower than usual, +2 is higher than usual)
float Respiration::getNormalizedAmplitude() const { 
  return amplitudeNormalizer;
}

 //Returns scaled breath amplitude (float between 0 and 1) : scaled by mapping and clamping normalized amplitude
float Respiration::getScaledAmplitude() const { 
  return _clampScaledAmplitude;
}

//Returns breath amplitude level indicator (float between 0 and 1)
 //(latest amplitudes are generally ===> 0 :  smaller than baseline, 0.5 : similar to baseline, 1 : larger than baseline)
float Respiration::getAmplitudeLevel() const { 
  return _amplitudeLevel;
}

//Returns breath amplitude rate of change (ADC points/minute)
float Respiration::getAmplitudeChange() const { 
  return _amplitudeRateOfChange;
}

//Returns breath amplitude coefficient of variation
float Respiration::getAmplitudeVariability() const { 
  return _amplitudeCoefficientOfVariation;
}

//Returns respiration interval (milliseconds between breath cycles)
unsigned long Respiration::getInterval() const { 
  return _interval;
}

//Returns respiration rate (respirations per minute)
float Respiration::getRpm() const { 
  return _rpm;
}

//Returns normalized respiration rate (target mean 0, stdDev 1) (example: -2 is lower than usual, +2 is higher than usual)
float Respiration::getNormalizedRpm() const { 
  return rpmNormalizer;
}

 //Returns scaled respiration rate (float between 0 and 1) : scaled by mapping and clamping normalized rpm
float Respiration::getScaledRpm() const { 
  return _clampScaledRpm;
}

//Returns repiration rate level indicator (float between 0 and 1)
 //(latest rpm values are generally ===> 0 :  smaller than baseline, 0.5 : similar to baseline, 1 : larger than baseline)
float Respiration::getRpmLevel() const { 
  return _rpmLevel;
}

//Returns respiration rate of change (rpm/minute)
float Respiration::getRpmChange() const { 
  return _rpmRateOfChange;
}

//Returns respiration rate coefficient of variation
float Respiration::getRpmVariability() const { 
  return _rpmCoefficientOfVariation;
}

void Respiration::begin() {
  reset();
}

void Respiration::step() {
  // sample at sampling rate
  if (sampleMetro) {
    sample();
  }
}


// Reads the signal and passes it to the signal processing functions
void Respiration::sample() {
  peakOrTrough(_value); // base signal processing
  amplitude(_value); // amplitude data processing
  rpm(); // respiration rate data processing
}

// Base temperature signal processing and peak detection
void Respiration::peakOrTrough(float value){
 value >> smoother >> normalizer; // smooth and normalize temperature signal

 if (calibrationTimer) {
    // PEAK DETECTION AND MIN MAX SCALED SIGNAL
    normalizer >> minMaxScaler;

    minMaxScaler >> minMaxScaledPeak; // peak detection
    minMaxScaler >> minMaxScaledTrough; // trough detection 

    _minMaxScaled = minMaxScaler; // scale to 0-1

    _exhale = minMaxScaledPeak ? 0 : minMaxScaledTrough ? 1 : _exhale; 
    // store true if exhaling (when trough is detected ; temperature is rising again)
 }
}

// Amplitude data processing
void Respiration::amplitude(float value){ 
   // declare and initialize local variables
  static float min = 0; // base signal value at lowest point in breath cycle
  static float max = 0; // base signal value at highest point in breath cycle
  static float amplitudes[numberOfCycles] = {}; // array of previous breath amplitudes
  static int amplitudeIndex = 0; // index 
  static float oldestAmplitude; // oldest breath amplitude in the array

  //AMPLITUDE 
  // find min and max values since last breath cycle
  if (value < min) min = value; 
  if (value > max) max = value;

  if (minMaxScaledPeak){  // on every exhale peak
    if(max > 0 && min > 0){ // if min and max temperatures are valid
      _amplitude = abs(max - min);  // calculate amplitude
    }
    min = 65535; // reset min to very high value
    max = 0; // reset max to very low value
  }

  if(_amplitude >0){ // if amplitude is valid 
  // NORMALIZED AMPLITUDE
    _amplitude >> amplitudeSmoother >> amplitudeNormalizer; // smooth and normalize amplitude
 
  // SCALED AMPLITUDE (clamped from normalized)
   _clampScaledAmplitude = mapTo01(amplitudeNormalizer, fromMinStdDev, fromMaxStdDev, CONSTRAIN);
   // scale amplitude by mapping and clamping normalized amplitude

    //AMPLITUDE RATE OF CHANGE
    if (minMaxScaledPeak) { // on every exhale peak
    // Circular buffer array of breath amplitudes
      oldestAmplitude = amplitudes[amplitudeIndex];
      amplitudes[amplitudeIndex] = _amplitude;
      amplitudeIndex = (amplitudeIndex + 1) % numberOfCycles;

      if(oldestAmplitude > 0){ // if oldest amplitude is valid
      _amplitudeRateOfChange = (_amplitude - oldestAmplitude)/_millisPassed * 60000; // calculate rate of change (ADC points/minute)
     }
    _amplitudeRateOfChange >> amplitudeRateOfChangeSmoother; // smooth rate of change
    }
    _amplitudeRateOfChange = amplitudeRateOfChangeSmoother;
    
    //AMPLITUDE VARIABILITY
    _amplitude >> normalizerForAmplitudeVariability; // pipe amplitude into a normalizer to access standard deviation and mean stats
    if(oldestAmplitude>0){ // if oldest amplitude is valid
    _amplitudeCoefficientOfVariation = (normalizerForAmplitudeVariability.stdDev() / normalizerForAmplitudeVariability.mean())*100;
    // calculate coefficient of variation
 }
}
  // //AMPLITUDE Level
    _amplitudeLevel = _clampScaledAmplitude >> amplitudeLevelSmoother; // smooth normalized amplitude
}

// Respiration rate data processing
void Respiration::rpm(){ 
  // declare and initialize local variables
    static unsigned long intervalChrono = millis(); // respiration interval chronometer (ms) 
    static int intervalIndex = 0; // index
    static unsigned long oldestInterval; // oldest breath interval in the array

  //INTERVAL
  if (minMaxScaledPeak){ // on every exhale peak
    if ((millis() - intervalChrono) >= 300){ // if interval is greater than 300ms (to prevent errors due to noise)
    _interval = millis() - intervalChrono; // calculate interval between current and previous exhale peak
    _rpm = 60000 /_interval;  // calculate breath rate (respirations per minute)
    }
    intervalChrono = millis(); // restart interval chronometer
  }

if (_interval > 0){ // if interval is valid
      //RPM + NORMALIZED RPM 
      _rpm >> rpmSmoother >> rpmNormalizer; // smooth and normalize rpm

      //SCALED RPM (clamped from normalized)
      _clampScaledRpm = mapTo01(rpmNormalizer, fromMinStdDev, fromMaxStdDev, CONSTRAIN);

  if (minMaxScaledPeak){ // on every exhale peak
    // Circular buffer array of breath intervals
      oldestInterval = intervals[intervalIndex];
      intervals[intervalIndex] = _interval;
      intervalIndex = (intervalIndex + 1) % numberOfCycles;

      // Calculate sum of intervals
      _millisPassed = 0;
      for (int i = 0; i<numberOfCycles; i++)
        _millisPassed += intervals[i];

  //RPM Change
    if (oldestInterval >0){ // if oldest interval is valid
      _rpmRateOfChange = (60000.0/_interval - 60000.0/oldestInterval)/_millisPassed * 60000; // calculate rate of change (rpm/minute);
    }
  _rpmRateOfChange >> rpmRateOfChangeSmoother; // smooth rate of change
  }
  _rpmRateOfChange = rpmRateOfChangeSmoother;


  //RPM VARIABILITY
  _rpm >> normalizerForRpmVariability; // pipe rpm into a normalizer to access standard deviation and mean stats
  if (oldestInterval >0){ // if oldest interval is valid
  _rpmCoefficientOfVariation = (normalizerForRpmVariability.stdDev() / normalizerForRpmVariability.mean())*100;
  // calculate coefficient of variation
  }
}
  //RPM Level
  _rpmLevel = _clampScaledRpm >> rpmLevelSmoother; // smooth normalized rpm
}

}
