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

Respiration::Respiration(uint8_t pin, unsigned long rate) :
  _pin(pin),
   ADS(pin),                     // 0x49 is the I2C address we chose (see ADS1115 datasheet for specifications)
  thermistor(),                  // thermistor
  normalizer(normalizerMean, normalizerStdDev, normalizerTimeWindow), 
  amplitudeNormalizer(normalizerMean, normalizerStdDev, amplitudeNormalizerTimeWindow),
  amplitudeVariabilityNormalizer(normalizerMean, normalizerStdDev, amplitudeVariabilityNormalizerTimeWindow),
  rpmNormalizer(normalizerMean, normalizerStdDev, rpmNormalizerTimeWindow),
  rpmVariabilityNormalizer(normalizerMean, normalizerStdDev, rpmVariabilityNormalizerTimeWindow),
  normalizedPeak(normalizedPeakThreshold, PEAK_MAX),
  normalizedTrough(normalizedTroughThreshold, PEAK_MIN),  
  minMaxScaledPeak(minMaxScaledPeakThreshold, PEAK_MAX),
  minMaxScaledTrough(minMaxScaledTroughThreshold, PEAK_MIN),
  smoother(smootherFactor),
  amplitudeSmoother(amplitudeSmootherFactor),
  amplitudeLevelSmoother(amplitudeLevelSmootherFactor),
  rpmSmoother(rpmSmootherFactor),
  rpmLevelSmoother(rpmLevelSmootherFactor),
  minMaxScaler(),
  amplitudeMinMaxScaler(),
  rpmMinMaxScaler(),
  _temperature(25),
  _adcValue(13000),
  _clampScaled(0.5),
  _minMaxScaled(0.5),
  _normalizedExhale(0),
  _minMaxScaledExhale(0),
  _amplitude(0.3),
  _clampScaledAmplitude(0.5),
  _minMaxScaledAmplitude(0.5),
  _amplitudeLevel(0.5),
  _amplitudeRoChange(),
  _amplitudeCV(0),
  _interval(0),
  _rpm(12),
  _clampScaledRpm(0.5),
  _minMaxScaledRpm(0.5),
  _rpmLevel(0.5),
  _rpmRoChange(0.5),
  _rpmCV(0),
  _min(),
  _max(),
  _millisPassed(0)
{
  setSampleRate(rate);
  reset();
}

void Respiration::reset() {
  Wire.begin();
  Wire.setClock(400000);   

  ADS.begin();                  // external ADC
  ADS.setMode(0);               // continuous mode
  ADS.readADC(_pin);            // first reading    

  prevSampleMicros = micros();

  //set peak detector thresholds
  normalizedPeak.reloadThreshold(normalizedPeakReloadThreshold);
  normalizedPeak.fallbackTolerance(normalizedPeakFallbackThreshold);
  normalizedTrough.reloadThreshold(normalizedTroughReloadThreshold);
  normalizedTrough.fallbackTolerance(normalizedTroughFallbackThreshold);

  minMaxScaledPeak.reloadThreshold(minMaxScaledPeakReloadThreshold);
  minMaxScaledPeak.fallbackTolerance(minMaxScaledPeakFallbackThreshold);
  minMaxScaledTrough.reloadThreshold(minMaxScaledTroughReloadThreshold);
  minMaxScaledTrough.fallbackTolerance(minMaxScaledTroughFallbackThreshold);

  minMaxScaler.timeWindow(normalizerTimeWindow);
  amplitudeMinMaxScaler.timeWindow(amplitudeNormalizerTimeWindow);
  rpmMinMaxScaler.timeWindow(rpmNormalizerTimeWindow);

  // Perform one update.
  sample();
}

void Respiration::setSampleRate(unsigned long rate) {
  sampleRate = rate;
  microsBetweenSamples = 1000000UL / sampleRate;  //
}

void Respiration::update() {
  unsigned long t = micros();
  if (t - prevSampleMicros >= microsBetweenSamples) {
    // Perform updates.
    sample();
    prevSampleMicros = t;
  }
}

uint16_t Respiration::getRaw()  const {
return _adcValue ;
}

float Respiration::getTemperature()  const {
return _temperature ;
}

void Respiration::sample() {
  _adcValue = ADS.getValue();
  _temperature = thermistor.readTemp(_adcValue);

  peakOrTrough(_temperature);
  amplitude(_temperature);
  rpm();
}

void Respiration::peakOrTrough(float value){ // base temperature signal processing and peak detection
  // BASE TEMPERATURE SIGNAL + NORMALIZED 
  value >> smoother >> normalizer; //smooth and normalize base temperature signal

  // FROM NORMALIZED SIGNAL : PEAK DETECTION AND CLAMP SCALED SIGNAL
  normalizer >> normalizedPeak; // detect max peak (exhale)
  normalizer >> normalizedTrough; // detect min trough (inhale)
  // store true if exhaling (0 = inhale / 1 = exhale)
  _normalizedExhale = normalizedPeak ? 1 : normalizedTrough ? 0 : _normalizedExhale; 

  _clampScaled = mapTo01(normalizer, fromMinStdDev, fromMaxStdDev,CONSTRAIN);

  // MINMAX SCALED SIGNAL : PEAK DETECTION AND MIN MAX SCALED SIGNAL
  _minMaxScaled = normalizer >> minMaxScaler;
  minMaxScaler >> minMaxScaledPeak;
  minMaxScaler>> minMaxScaledTrough;
  _minMaxScaledExhale = minMaxScaledPeak ? 1 : minMaxScaledTrough ? 0 : _minMaxScaledExhale; // store true if exhaling (0 = inhale / 1 = exhale)
}

void Respiration::amplitude(float value){ // amplitude data processing
  // declare and initialize local variables
  static float min ; // base signal value at lowest point in breath cycle
  static float max ; // base signal value at highest point in breath cycle
  static float amplitudes[numberOfCycles] = {}; // array of previous breath amplitudes
  static int index = 0; // index 

  //AMPLITUDE 
  if (minMaxScaledPeak){  
    max = value; // base signal value at highest point in breath cycle
    if(max != 0 && min != 0) {
      _amplitude = abs(max - min);
    }
    _min = min;
    _max = max;
  }
  if (minMaxScaledTrough) min = value; // base signal value at lowest point in breath cycle

  // NORMALIZED AMPLITUDE
  if(min !=0 && max != 0) {
    _amplitude >> amplitudeSmoother >> amplitudeNormalizer; // smooth and normalize amplitude
  }
 
  // SCALED AMPLITUDE (clamped from normalized)
  _clampScaledAmplitude = mapTo01(amplitudeNormalizer, fromMinStdDev, fromMaxStdDev, CONSTRAIN);

  // SCALED AMPLITUDE (MinMaxScaler)
  _minMaxScaledAmplitude = amplitudeNormalizer >> amplitudeMinMaxScaler;
 
  //AMPLITUDE VARIABILITY
    _amplitude >> amplitudeVariabilityNormalizer; // pipe amplitude into a normalizer with 30 second time window to access standard deviation and mean stats
    _amplitudeCV = (amplitudeVariabilityNormalizer.stdDev() / amplitudeVariabilityNormalizer.mean())*100;
    
  //AMPLITUDE Level
   _amplitudeLevel = _minMaxScaledAmplitude >> amplitudeLevelSmoother; // smooth normalized amplitude
  
  //AMPLITUDE RoChange && SMAChange
    if (minMaxScaledPeak) { // on every exhale peak
      float oldestBreath = amplitudes[index];
      amplitudes[index] = amplitudeSmoother;
      index = (index + 1) % numberOfCycles;

      _amplitudeRoChange = (_amplitude - oldestBreath)/_millisPassed * 60000; // breath amplitude rate of change (Celcius/minute)
    }
}



void Respiration::rpm(){ // respiration rate data processing (respirations per minute)
  static uint16_t interval = 0; // respiration interval (ms)
    static uint16_t intervalChrono = millis(); // respiration interval chronometer (ms) 
    static int intervalIndex = 0; // index
    static float rpms[numberOfCycles] = {}; // array of previous breath rates
    static int rpmIndex = 0; // index 


  //INTERVAL
  if (minMaxScaledPeak){ // on every exhale peak
    interval = millis() - intervalChrono; // calculate interval between current and previous exhale peak
    intervalChrono = millis(); // restart interval chronometer
    if (interval >= 30) _rpm = 60000 / interval; // calculate rpm from interval
    // minimal interval condition to bypass noise errors 

    _millisPassed = std::accumulate(std::begin(intervals), std::end(intervals), 0); // sum of intervals[]

      intervals[intervalIndex] = interval;
      intervalIndex = (intervalIndex + 1) % numberOfCycles;

  //RPM Change
      int oldestRpm = rpms[rpmIndex];
      rpms[rpmIndex] = rpmSmoother;
      rpmIndex = (rpmIndex + 1) % numberOfCycles;
      _rpmRoChange = (rpmSmoother - oldestRpm)/_millisPassed * 60000; // breath rate of change (Celcius/minute) - previous oldestInterval;
  }
  _interval = interval;


  //RPM + NORMALIZED RPM 
  _rpm >> rpmSmoother >> rpmNormalizer; // smooth and normalize rpm

  //SCALED RPM (clamped from normalized)
  _clampScaledRpm = mapTo01(rpmNormalizer, fromMinStdDev, fromMaxStdDev, CONSTRAIN);

  //SCALED RPM (MinMaxScaler)
  _minMaxScaledRpm = rpmNormalizer >> rpmMinMaxScaler;

  //RPM VARIABILITY
  _rpm >> rpmVariabilityNormalizer; // pipe rpm into a normalizer with 30 second time window to access standard deviation and mean stats
   _rpmCV = (rpmVariabilityNormalizer.stdDev() / rpmVariabilityNormalizer.mean())*100;
    
  //RPM Level
  _rpmLevel = _minMaxScaledRpm >> rpmLevelSmoother; // smooth normalized rpm
}

//returns normalized temperature signal (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
float Respiration::getNormalized() {  
  return normalizer;
} 

//returns scaled temperature signal (returns a float between 0 and 1) : scaled by mapping and clamping normalized signal
float Respiration::getClampScaled() { 
  return _clampScaled;
} 

//returns scaled temperature signal (default : returns a float between 0 and 1) : scaled by MinMaxScaler
float Respiration::getMinMaxScaled() { 
  return _minMaxScaled;
}

//returns true if exhaling (from normalized value)
bool Respiration::normalizedIsExhaling() const{ 
  return _normalizedExhale;
}

//returns true if exhaling (from MinMaxScaler scaled value)
bool Respiration::minMaxScaledIsExhaling() const{ 
  return _minMaxScaledExhale;
}

//returns breah amplitude (temperature difference between breath cycle peak and trough)
float Respiration::getTemperatureAmplitude() const{ 
  return _amplitude;
}

 //returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
float Respiration::getNormalizedAmplitude(){ 
  return amplitudeNormalizer;
}

 //returns scaled breath amplitude (returns a float between 0 and 1) : scaled by mapping and clamping normalized amplitude
float Respiration::getClampScaledAmplitude() { 
  return _clampScaledAmplitude;
}

//returns scaled breath amplitude (default : returns a float between 0 and 1) : scaled by MinMaxScaler
float Respiration::getMinMaxScaledAmplitude() { 
  return _minMaxScaledAmplitude;
}

//returns breath amplitude level indicator (normalized amplitude smoothed and scaled between 0 and 1)
//(0 : smaller than baseline, 0.5 : no significant change from baseline, 1 : larger than baseline)
float Respiration::getAmplitudeLevel() const{ 
  return _amplitudeLevel;
}

//returns breath amplitude rate of change (RoC)
float Respiration::getAmplitudeChange() const{ 
  return _amplitudeRoChange;
}

//returns breath amplitude coefficient of variation
float Respiration::getAmplitudeVariability() const{ 
  return _amplitudeCV;
}

//returns respiration interval (milliseconds between breath cycles)
float Respiration::getInterval() const{ 
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
float Respiration::getClampScaledRpm() { 
  return _clampScaledRpm;
}

//returns scaled respiration rate (default : returns a float between 0 and 1) : scaled by MinMaxScaler
float Respiration::getMinMaxScaledRpm() { 
  return _minMaxScaledRpm;
}

//returns repiration rate level indicator (0 : slower than baseline, 0.5 : no significant change from baseline, 1 : faster than baseline)
float Respiration::getRpmLevel() const{ 
  return _rpmLevel;
}

//returns respiration rate change indicator (difference between calculated respiration rate of current and previous breath cycle)
float Respiration::getRpmChange() const{ 
  return _rpmRoChange;
}

//returns respiration rate coefficient of variation
float Respiration::getRpmVariability() const{ 
  return _rpmCV;
}

float Respiration::getMin() { 
  return _min;
}

float Respiration::getMax(){ 
  return _max;
}
