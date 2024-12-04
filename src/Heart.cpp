/*
 * Heart.cpp
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
#include "Heart.h"

//=============================================CONSTRUCTORS=============================================//
// CONSTRUCTOR
Heart::Heart(unsigned long rate){
    initialize(rate);
} 

//=================================================SET=============================================//
void Heart::initialize(unsigned long rate){

    heartThresh(0.25, 0.4);              // if signal does not fall below (low, high) bounds than signal is ignored
    heartMinMaxSmoothing(0.1);
    heartSensorAmplitudeLop(0.001);
    heartSensorBpmLop(0.001);
    heartSensorAmplitudeLopValueMinMaxSmoothing(0.001);
    heartSensorBpmLopValueMinMaxSmoothing(0.001);

    heartMinMax.reset();
    heartSensorAmplitudeLop.reset();
    heartSensorBpmLop.reset();
    heartSensorAmplitudeLopValueMinMax.reset();
    heartSensorBpmLopValueMinMax.reset();

    heartSensorSignal = heartSensorFiltered = heartSensorAmplitude = 0;
    bpmChronoStart = millis();

    bpm = 60;
    beat = false;

    prevSampleMicros = micros();

    setSampleRate(rate);
}

void Heart::setAmplitudeSmoothing(float smoothing)
{
    heartSensorAmplitudeLop.setSmoothing(smoothing);
}

void Heart::setBpmSmoothing(float smoothing)
{
    heartSensorBpmLop.setSmoothing(smoothing);
}

void Heart::setAmplitudeMinMaxSmoothing(float smoothing)
{
    heartSensorAmplitudeLopValueMinMaxSmoothing = constrain(smoothing, 0, 1);
}

void Heart::setBpmMinMaxSmoothing(float smoothing)
{
    heartSensorBpmLopValueMinMaxSmoothing = constrain(smoothing, 0, 1);
}

void Heart::setMinMaxSmoothing(float smoothing)
{
    heartMinMaxSmoothing = constrain(smoothing, 0, 1);
}

void Heart::setSampleRate(unsigned long rate) {
    _sampleRate = rate;
    microsBetweenSamples = 1000000UL / _sampleRate;
}

void Heart::update(float signal) {
    // unsigned long t = micros();
    unsigned long t = micros();
    if (t - prevSampleMicros >= microsBetweenSamples) {
        // Perform updates.
        sample();
        prevSampleMicros = t;
    }
}

float Heart::getNormalized() const {
    return heartSensorFiltered;
}

float Heart::amplitudeChange() const {
    return heartSensorAmplitudeLopValueMinMaxValue;
}

float Heart::bpmChange() const {
    return heartSensorBpmLopValueMinMaxValue;
}

bool Heart::beatDetected() const {
    return beat;
}

float Heart::getBPM() const {
    return bpm;
}

int Heart::getRaw() const {
    return heartSensorSignal;
}

void Heart::sample(float signal) {
    // Read analog value if needed.
    heartSensorSignal = signal;

    heartSensorFiltered = heartMinMax.filter(heartSensorSignal);
    heartSensorAmplitude = heartMinMax.getMax() - heartMinMax.getMin();
    heartMinMax.adapt(heartMinMaxSmoothing); // APPLY A LOW PASS ADAPTION FILTER TO THE MIN AND MAX

    heartSensorAmplitudeLopValue = heartSensorAmplitudeLop.filter(heartSensorAmplitude);
    heartSensorBpmLopValue =  heartSensorBpmLop.filter(bpm);

    heartSensorAmplitudeLopValueMinMaxValue = heartSensorAmplitudeLopValueMinMax.filter(heartSensorAmplitudeLopValue);
    heartSensorAmplitudeLopValueMinMax.adapt(heartSensorAmplitudeLopValueMinMaxSmoothing);
    heartSensorBpmLopValueMinMaxValue = heartSensorBpmLopValueMinMax.filter(heartSensorBpmLopValue);
    heartSensorBpmLopValueMinMax.adapt(heartSensorBpmLopValueMinMaxSmoothing);

    beat = heartThresh.detect(heartSensorFiltered);

    if ( beat ) {
        unsigned long ms = millis();
        float temporaryBpm = 60000. / (ms - bpmChronoStart);
        bpmChronoStart = ms;
        if ( temporaryBpm > 30 && temporaryBpm < 200 ) // make sure the BPM is within bounds
            bpm = temporaryBpm;
    }
}
