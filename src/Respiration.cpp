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

Respiration::Respiration(uint8_t pin, unsigned long rate, ADC_RESOLUTION resolution) :
  _pin(pin),
  thermistor(resolution),                  // thermistor
  normalizer(normalizerMean, normalizerStdDev, normalizerTimeWindow), 
  amplitudeNormalizer(normalizerMean, normalizerStdDev, amplitudeNormalizerTimeWindow),
  normalizerForAmplitudeVariability(normalizerMean, normalizerStdDev, normalizerForAmplitudeVariabilityTimeWindow),
  rpmNormalizer(normalizerMean, normalizerStdDev, rpmNormalizerTimeWindow),
  normalizerForRpmVariability(normalizerMean, normalizerStdDev, normalizerForRpmVariabilityTimeWindow),
  minMaxScaledPeak(minMaxScaledPeakThreshold, PEAK_MAX),
  minMaxScaledTrough(minMaxScaledTroughThreshold, PEAK_MIN),
  smoother(smootherFactor),
  amplitudeSmoother(amplitudeSmootherFactor),
  amplitudeLevelSmoother(amplitudeLevelSmootherFactor),
  amplitudeRateOfChangeSmoother(amplitudeRateOfChangeSmootherFactor),
  rpmSmoother(rpmSmootherFactor),
  rpmLevelSmoother(rpmLevelSmootherFactor),
  rpmRateOfChangeSmoother(rpmRateOfChangeSmootherFactor),
  minMaxScaler(),
  _temperature(25),
  _adcValue(13000),
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
  _min(-FLT_MIN),
  _max(-FLT_MIN),
  _millisPassed(0),
  _getExternalADCValue(nullptr)
{
  setSampleRate(rate);
  reset();
}

Respiration::Respiration(unsigned long (*getExternalADCValue)(), unsigned long rate, ADC_RESOLUTION resolution) :
  _pin(0),
  thermistor(resolution),                  // thermistor
  normalizer(normalizerMean, normalizerStdDev, normalizerTimeWindow), 
  amplitudeNormalizer(normalizerMean, normalizerStdDev, amplitudeNormalizerTimeWindow),
  normalizerForAmplitudeVariability(normalizerMean, normalizerStdDev, normalizerForAmplitudeVariabilityTimeWindow),
  rpmNormalizer(normalizerMean, normalizerStdDev, rpmNormalizerTimeWindow),
  normalizerForRpmVariability(normalizerMean, normalizerStdDev, normalizerForRpmVariabilityTimeWindow),
  minMaxScaledPeak(minMaxScaledPeakThreshold, PEAK_MAX),
  minMaxScaledTrough(minMaxScaledTroughThreshold, PEAK_MIN),
  smoother(smootherFactor),
  amplitudeSmoother(amplitudeSmootherFactor),
  amplitudeLevelSmoother(amplitudeLevelSmootherFactor),
  amplitudeRateOfChangeSmoother(amplitudeRateOfChangeSmootherFactor),
  rpmSmoother(rpmSmootherFactor),
  rpmLevelSmoother(rpmLevelSmootherFactor),
  rpmRateOfChangeSmoother(rpmRateOfChangeSmootherFactor),
  minMaxScaler(),
  _temperature(25),
  _adcValue(13000),
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
  _min(-FLT_MIN),
  _max(-FLT_MIN),
  _millisPassed(0),
  _getExternalADCValue(getExternalADCValue)
{
  setSampleRate(rate);
  reset();
}

void Respiration::reset() {
  Wire.begin();
  Wire.setClock(400000);   

  //set peak detector thresholds
  minMaxScaledPeak.reloadThreshold(minMaxScaledPeakReloadThreshold);
  minMaxScaledPeak.fallbackTolerance(minMaxScaledPeakFallbackThreshold);
  minMaxScaledTrough.reloadThreshold(minMaxScaledTroughReloadThreshold);
  minMaxScaledTrough.fallbackTolerance(minMaxScaledTroughFallbackThreshold);

  minMaxScaler.timeWindow(normalizerTimeWindow);

  // Perform one update.
  sample();
}

void Respiration::setSampleRate(unsigned long rate) {
  sampleMetro.frequency(rate);
}

void Respiration::update() {
  if (sampleMetro) {
    sample();
  }
}

unsigned long Respiration::getRaw()  const {
return _adcValue ;
}

float Respiration::getTemperature()  const {
return _temperature ;
}

void Respiration::sample() {
  if(_getExternalADCValue){
    _adcValue = _getExternalADCValue();
  } else {
    _adcValue = analogRead(_pin);
  }

  if(_adcValue >= 0){
    _temperature = thermistor.readTemp(_adcValue);
    if(_temperature > -273){
     peakOrTrough(_temperature);
     amplitude(_temperature);
     rpm();
    }
  }
}

void Respiration::peakOrTrough(float value){ // base temperature signal processing and peak detection
 value >> smoother >> normalizer;

  // PEAK DETECTION AND MIN MAX SCALED SIGNAL
  _minMaxScaled = normalizer >> minMaxScaler;
  minMaxScaler >> minMaxScaledPeak;
  minMaxScaler>> minMaxScaledTrough;
  _exhale = minMaxScaledPeak ? 0 : minMaxScaledTrough ? 1 : _exhale; // store true if exhaling (0 = inhale / 1 = exhale)
}

void Respiration::amplitude(float value){ // amplitude data processing
   // declare and initialize local variables
  static float min = -273; // base signal value at lowest point in breath cycle
  static float max = -273; // base signal value at highest point in breath cycle
  static float amplitudes[numberOfCycles] = {}; // array of previous breath amplitudes
  static int amplitudeIndex = 0; // index 
  static float oldestAmplitude;

  // update min and max
  if (value < min) min = value;
  if (value > max) max = value;

  //AMPLITUDE 
  if (minMaxScaledPeak){  
    if(max > -273 && min > -273){
      _amplitude = abs(max - min); 
    }
    min = 273; // reset min to current value
    max = -273; // reset max to current value
  }

  if(_amplitude >0){
  // // NORMALIZED AMPLITUDE
    _amplitude >> amplitudeSmoother >> amplitudeNormalizer; // smooth and normalize amplitude
 
  // // SCALED AMPLITUDE (clamped from normalized)
   _clampScaledAmplitude = mapTo01(amplitudeNormalizer, fromMinStdDev, fromMaxStdDev, CONSTRAIN);

  
    //AMPLITUDE Rate Of Change
    if (minMaxScaledPeak) { // on every exhale peak
      oldestAmplitude = amplitudes[amplitudeIndex];
      amplitudes[amplitudeIndex] = _amplitude;
      amplitudeIndex = (amplitudeIndex + 1) % numberOfCycles;

      if(oldestAmplitude > 0){
      _amplitudeRateOfChange = (_amplitude - oldestAmplitude)/_millisPassed * 60000; // breath amplitude rate of change (Celcius/minute)
     }
    _amplitudeRateOfChange >> amplitudeRateOfChangeSmoother;
    }
    _amplitudeRateOfChange = amplitudeRateOfChangeSmoother;
    
    // //AMPLITUDE VARIABILITY
    _amplitude >> normalizerForAmplitudeVariability; // pipe amplitude into a normalizer with 30 second time window to access standard deviation and mean stats
    if(oldestAmplitude>0){
    _amplitudeCoefficientOfVariation = (normalizerForAmplitudeVariability.stdDev() / normalizerForAmplitudeVariability.mean())*100;
 }
}
  // //AMPLITUDE Level
    _amplitudeLevel = _clampScaledAmplitude >> amplitudeLevelSmoother; // smooth normalized amplitude
}

void Respiration::rpm(){ // respiration rate data processing (respirations per minute)
    static unsigned long intervalChrono = millis(); // respiration interval chronometer (ms) 
    static int intervalIndex = 0; // index
    static float rpms[numberOfCycles] = {}; // array of previous breath rates
    static unsigned long oldestInterval;


  //INTERVAL
  if (minMaxScaledPeak){ // on every exhale peak
    if ((millis() - intervalChrono) >= 300){
    _interval = millis() - intervalChrono; // calculate interval between current and previous exhale peak
    _rpm = 60000 /_interval; 
    }
    intervalChrono = millis(); // restart interval chronometer
  }

if (_interval > 0){
      //RPM + NORMALIZED RPM 
      _rpm >> rpmSmoother >> rpmNormalizer; // smooth and normalize rpm

      //SCALED RPM (clamped from normalized)
      _clampScaledRpm = mapTo01(rpmNormalizer, fromMinStdDev, fromMaxStdDev, CONSTRAIN);

  if (minMaxScaledPeak){ // on every exhale peak
      oldestInterval = intervals[intervalIndex];
      intervals[intervalIndex] = _interval;
      intervalIndex = (intervalIndex + 1) % numberOfCycles;

      _millisPassed = std::accumulate(std::begin(intervals), std::end(intervals), 0); // sum of intervals[]

  //RPM Change
    if (oldestInterval >0){
      _rpmRateOfChange = (60000.0/_interval - 60000.0/oldestInterval)/_millisPassed * 60000; // breath rate of change (Celcius/minute) - previous oldestInterval;
    }
  _rpmRateOfChange >> rpmRateOfChangeSmoother;
  }
  _rpmRateOfChange = rpmRateOfChangeSmoother;


  //RPM VARIABILITY
  _rpm >> normalizerForRpmVariability; // pipe rpm into a normalizer with 30 second time window to access standard deviation and mean stats
  if (oldestInterval >0){
  _rpmCoefficientOfVariation = (normalizerForRpmVariability.stdDev() / normalizerForRpmVariability.mean())*100;
  }
}
  //RPM Level
  _rpmLevel = _clampScaledRpm >> rpmLevelSmoother; // smooth normalized rpm
}

//returns normalized temperature signal (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
float Respiration::getNormalized() {  
  return normalizer;
} 

//returns scaled temperature signal (default : returns a float between 0 and 1) : scaled by MinMaxScaler
float Respiration::getScaled() { 
  return _minMaxScaled;
}

//returns true if exhaling (from MinMaxScaler scaled value)
bool Respiration::isExhaling() const{ 
  return _exhale;
}

//returns breah amplitude (temperature difference between breath cycle peak and trough)
float Respiration::getTemperatureAmplitude() const{ 
  if(_amplitude>=0){
      return _amplitude;
    } else {
      return 0;
    }
}

 //returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
float Respiration::getNormalizedAmplitude(){ 
  return amplitudeNormalizer;
}

 //returns scaled breath amplitude (returns a float between 0 and 1) : scaled by mapping and clamping normalized amplitude
float Respiration::getScaledAmplitude() { 
  return _clampScaledAmplitude;
}

//returns breath amplitude level indicator (normalized amplitude smoothed and scaled between 0 and 1)
//(0 : smaller than baseline, 0.5 : no significant change from baseline, 1 : larger than baseline)
float Respiration::getAmplitudeLevel() const{ 
  return _amplitudeLevel;
}

//returns breath amplitude rate of change (RoC)
float Respiration::getAmplitudeChange() const{ 
  return _amplitudeRateOfChange;
}

//returns breath amplitude coefficient of variation
float Respiration::getAmplitudeVariability() const{ 
  return _amplitudeCoefficientOfVariation;
}

//returns respiration interval (milliseconds between breath cycles)
unsigned long Respiration::getInterval() const{ 
  return _interval;
}

//returns respiration rate (respirations per minute)
float Respiration::getRpm() const{ 
  return _rpm;
}

//returns normalized respiration rate (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
float Respiration::getNormalizedRpm(){ 
  return rpmNormalizer;
}

 //returns scaled respiration rate (returns a float between 0 and 1) : scaled by mapping and clamping normalized rpm
float Respiration::getScaledRpm() { 
  return _clampScaledRpm;
}

//returns repiration rate level indicator (0 : slower than baseline, 0.5 : no significant change from baseline, 1 : faster than baseline)
float Respiration::getRpmLevel() const{ 
  return _rpmLevel;
}

//returns respiration rate change indicator (difference between calculated respiration rate of current and previous breath cycle)
float Respiration::getRpmChange() const{ 
  return _rpmRateOfChange;
}

//returns respiration rate coefficient of variation
float Respiration::getRpmVariability() const{ 
  return _rpmCoefficientOfVariation;
}

