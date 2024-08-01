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
  normalizerAmplitude(normalizerMean, normalizerStdDev, normalizerAmplitudeTimeWindow),
  normalizerAmplitudeChange(normalizerMean, normalizerStdDev, normalizerAmplitudeChangeTimeWindow),
  normalizerAmplitudeVariability(normalizerMean, normalizerStdDev, normalizerAmplitudeVariabilityTimeWindow),
  normalizerRpm(normalizerMean, normalizerStdDev, normalizerRpmTimeWindow),
  normalizerRpmChange(normalizerMean, normalizerStdDev, normalizerRpmChangeTimeWindow),
  normalizerRpmVariability(normalizerMean, normalizerStdDev, normalizerRpmVariabilityTimeWindow),
  normalizerFlowRate(normalizerMean, normalizerStdDev, normalizerFlowRateTimeWindow),
  peak(peakThreshold, PEAK_MAX),
  trough(troughThreshold, PEAK_MIN),
  flowRatePeak(flowRatePeakThreshold, PEAK_MAX),
  smoother(smootherFactor),
  smootherAmplitude(smootherAmplitudeFactor),
  smootherAmplitudeChange(smootherAmplitudeChangeFactor),
  smootherRpm(smootherRpmFactor),
  smootherRpmChange(smootherRpmChangeFactor),
  smootherFlowRate(smootherFlowRateFactor),
  scaler(), 
  scalerAmplitudeChange(),
  scalerAmplitudeVariability(), 
  scalerRpmChange(),
  scalerRpmVariability(),
  flowRateMetro(flowRateMetroTimer),
  _temperature(25),
  _adcValue(13000),
  _state(0),
  _amplitude(0.3),
  _amplitudeChange(0),
  _amplitudeDelta(0),
  _amplitudeCV(0),
  _interval(0),
  _rpm(12),
  _rpmChange(0),
  _rpmDelta(0),
  _rpmCV(0),
  _flowRateSurges(0),
  _flowRateVariability(0)
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

  //set scaler time windows
  scaler.timeWindow(scalerTimeWindow);
  scalerAmplitudeChange.timeWindow(scalerAmplitudeChangeTimeWindow);
  scalerRpmChange.timeWindow(scalerRpmChangeTimeWindow);

  //set peak detector thresholds
  peak.reloadThreshold(peakReloadThreshold);
  peak.fallbackTolerance(peakFallbackThreshold);
  trough.reloadThreshold(troughReloadThreshold);
  trough.fallbackTolerance(troughFallbackThreshold);

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

  state(_temperature);
  amplitude(_temperature);
  rpm();
  flowRate(_temperature);
}

void Respiration::state(float value){ // base temperature signal processing and peak detection
  value >> smoother >> normalizer >> scaler; //smooth, normalize and scale base temperature signal
  scaler >> peak; // detect max peak (exhale)
  scaler >> trough; // detect min peak (inhale)
  _state = peak ? 1 : trough ? 0 : _state; // store state (0 = inhale / 1 = exhale)
}

void Respiration::amplitude(float value){ // amplitude data processing
  // declare and initialize local variables
  static float min = 0; // base signal value at lowest point in breath cycle
  static float max = 0; // base signal value at highest point in breath cycle
  static float pAmplitude = 0; // previous amplitude to determine ampliude difference

  //AMPLITUDE + NORMALIZED AMPLITUDE
  if (peak){  
    max = value; // base signal value at highest point in breath cycle
    _amplitude = abs(max - min); // calculate absolute amplitude 
  }
  if (trough) min = value; // base signal value at lowest point in breath cycle
  _amplitude >> smootherAmplitude >> normalizerAmplitude; // smooth and normalize amplitude
  
  //AMPLITUDE VARIABILITY
    _amplitude >> normalizerAmplitudeVariability; // pipe amplitude into a normalizer with 30 second time window to access standard deviation and mean stats
    _amplitudeCV = (normalizerAmplitudeVariability.stdDev() / normalizerAmplitudeVariability.mean())*100;
    
  //AMPLITUDE CHANGE
   _amplitudeChange = normalizerAmplitude >> smootherAmplitudeChange >> scalerAmplitudeChange; // smooth and scale normalized amplitude
  
  //AMPLITUDE DELTA
    if (peak) { // on every exhale peak
        _amplitudeDelta = _amplitude - pAmplitude; // calculate the difference with previous amplitude *RAW OR NORMALIZED AMPLITUDE?
        pAmplitude = _amplitude; // store amplitude of latest breath cycle
    } 
}


void Respiration::rpm(){ // respiration rate data processing (respirations per minute)
  static uint16_t interval = 0; // respiration interval (ms)
    static uint16_t intervalChrono = millis(); // respiration interval chronometer (ms) 
    static float pRpm = 0; // previous rpm to determine rpm difference

  //RPM + NORMALIZED RPM
  if (peak){ // on every exhale peak
    interval = millis() - intervalChrono; // calculate interval between current and previous exhale peak
    intervalChrono = millis(); // restart interval chronometer
    if (interval >= 30) _rpm = 60000 / interval; // calculate rpm from interval
    // minimal interval condition to bypass noise errors 
  }
  _interval = interval;
  _rpm >> smootherRpm >> normalizerRpm; // smooth and normalize rpm

  //RPM VARIABILITY
  _rpm >> normalizerRpmVariability; // pipe rpm into a normalizer with 30 second time window to access standard deviation and mean stats
   _rpmCV = (normalizerRpmVariability.stdDev() / normalizerRpmVariability.mean())*100;
    
  //RPM CHANGE
  _rpmChange = normalizerRpm >> smootherRpmChange >> scalerRpmChange; // smooth and scale normalized rpm

  //RPM DELTA
  if(peak){ // on every exhale peak
      _rpmDelta = _rpm - pRpm; // calculate the difference with previous rpm *RAW OR NORMALIZED RPM?
      pRpm = _rpm; // store rpm of latest breath cycle
    }
}

void Respiration::flowRate(float value){ // flow rate data processing
  static float pValue = 30; // previous temperature signal value
    static float flowRate = 0.01; // flow rate (temperature difference in a given period of time)
  float surgeOutlierThreshold = 3;  // threshold for an outlier surge (big surge)
    static uint16_t surgeCounter = 0; // counter for number of surges during a breath cycle

  //FLOW RATE 
  if (flowRateMetro){ // frequency of temperature sampling for flow rate calculation
    flowRate = value - pValue; // calculate difference between current and previous temperature value
    flowRate >> smootherFlowRate >> normalizerFlowRate; // smooth and normalize flow rate
    pValue = value; // store temperature value of latest sampling 
  }

  //SURGES 
    _flowRateSurges = (normalizerFlowRate.isHighOutlier(flowRate, surgeOutlierThreshold)) ? 1: // if there is an increase outlier surge, _surges value is 2
          (normalizerFlowRate.isLowOutlier(flowRate, surgeOutlierThreshold)) ? -1:0; // if there is a decrease outlier surge, _surges value is -2
          // else, _surges value is 0

  //FLOW RATE VARIABILITY
    abs(normalizerFlowRate) >> flowRatePeak;
    if (flowRatePeak) surgeCounter ++; 
  if (peak){ // on every exhale peak
    _flowRateVariability = surgeCounter;
    surgeCounter = 0; // reset surge counter for next breath cycle
  } 
}

//returns normalized temperature signal (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
float Respiration::getNormalized() {  
  return normalizer;
} 

//returns scaled temperature signal (float between 0 and 1 : 0 is min value, 1 is max value)
float Respiration::getScaled(){ 
  return scaler;
} 

//returns breath state (two states : 0 is inhale, 1 is exhale)
uint8_t Respiration::getState() const{ 
  return _state;
}

//returns breah amplitude (raw) (difference between ADC at breath cycle peak and trough)
float Respiration::getAmplitude() const{ 
  return _amplitude;
}

 //returns normalized breath amplitude (target mean 0, stdDev 1) (example: -2 is abnormally low, +2 is abnormally high)
float Respiration::getAmplitudeNormalized(){ 
  return normalizerAmplitude;
}

//returns breath amplitude change indicator (0 : smaller than baseline, 0.5 : no significant change from baseline, 1 : larger than baseline)
float Respiration::getAmplitudeChange() const{ 
  return _amplitudeChange;
}

//returns breath amplitude delta (difference between amplitude of current and previous breath cycle)
float Respiration::getAmplitudeDelta() const{ 
  return _amplitudeDelta;
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
float Respiration::getRpmNormalized(){ 
  return normalizerRpm;
}

//returns repiration rate change indicator (0 : slower than baseline, 0.5 : no significant change from baseline, 1 : faster than baseline)
float Respiration::getRpmChange() const{ 
  return _rpmChange;
}

//returns respiration rate delta (difference between calculated respiration rate of current and previous breath cycle)
float Respiration::getRpmDelta() const{ 
  return _rpmDelta;
}

//returns respiration rate coefficient of variation
float Respiration::getRpmVariability() const{ 
  return _rpmCV;
}

//returns flow rate 
// (flow rate : temperature difference in a given period of time) (delta between current and previous temperature value)
float Respiration::getFlowRateSurges() const{ 
  return _flowRateSurges;
}

//returns flow rate coefficient of variation
float Respiration::getFlowRateVariability() const{ 
  return _flowRateVariability;
}

