/*
 * Heart.h
 *
 * This class defines an object that can be used to gather information about
 * pulse as gathered through a photoplethysmograph such as the Pulse Sensor.
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

#include "Average.h"
#include "MinMax.h"
#include "Threshold.h"
#include "Lop.h"

#ifndef HEART_H_
#define HEART_H_

class Heart {
    
    // Analog pin the Heart sensor is connected to.
    uint8_t _pin;
    
    unsigned long bpmChronoStart;
    
    MinMax heartMinMax;
    Threshold heartThresh;
    float heartMinMaxSmoothing;
    
    Lop heartSensorAmplitudeLop;
    Lop heartSensorBpmLop;
    
    float heartSensorAmplitudeLopValue;
    
    float heartSensorBpmLopValue;
    MinMax heartSensorAmplitudeLopValueMinMax;
    float heartSensorAmplitudeLopValueMinMaxSmoothing;
    
    float heartSensorAmplitudeLopValueMinMaxValue;
    MinMax heartSensorBpmLopValueMinMax;
    float heartSensorBpmLopValueMinMaxSmoothing;
    
    float heartSensorBpmLopValueMinMaxValue;
    
    float heartSensorFiltered;
    float heartSensorAmplitude;
    
    float heartSensorReading;
    
    float bpm;  // this value is fed to initialize your BPM before a heartbeat is detected
    
    bool beat;
    
    // Sample rate in Hz.
    unsigned long sampleRate;
    
    // Internal use.
    unsigned long microsBetweenSamples;
    unsigned long prevSampleMicros;
    
public:
    Heart(uint8_t pin, unsigned long rate=200); // default samplerate is 200Hz
    virtual ~Heart() {}
    
    void setAmplitudeSmoothing(float smoothing);
    void setBpmSmoothing(float smoothing);
    void setAmplitudeMinMaxSmoothing(float smoothing);
    void setBpmMinMaxSmoothing(float smoothing);
    void setMinMaxSmoothing(float smoothing);
    
    /// Resets all values.
    void reset();
    
    /// Sets sample rate.
    void setSampleRate(unsigned long rate);
    
    /**
     * Reads the signal and perform filtering operations. Call this before
     * calling any of the access functions. This function takes into account
     * the sample rate.
     */
    void update();
    
    /// Get normalized heartrate signal.
    float getNormalized() const;
    
    /// Returns true if a beat was detected during the last call to update().
    bool beatDetected() const;
    
    /// Returns BPM (beats per minute).
    float getBPM() const;
    
    /// Returns raw signal as returned by analogRead().
    int getRaw() const;
    
    ///Returns the average amplitude of signal mapped between 0.0 and 1.0.
    /* For example, if amplitude is average, returns 0.5,
     * if amplitude is below average, returns < 0.5
     * if amplitude is above average, returns > 0.5.
     */
    float amplitudeChange() const;
    
    /// Returns the average bpm of signal mapped between 0.0 and 1.0.
    /* For example, if bpm is average, returns 0.5,
     * if bpm is below average, returns < 0.5
     * if bpm is above average, returns > 0.5.
     */
    float bpmChange() const;
    
    // Performs the actual adjustments of signals and filterings.
    // Internal use: don't use directly, use update() instead.
    void sample();
};

#endif