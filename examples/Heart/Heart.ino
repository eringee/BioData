// This example demonstrates beating an LED on heartbeat, and data relevant to Heart
// for more info see README at https://github.com/eringee/BioData/
/******************************************************
copyright Erin Gee 2017
Authors Erin Gee // Martin Peach // Thomas Ouellet-Fredericks
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as published by
the Free Software Foundation.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    For more details: <http://www.gnu.org/licenses/>.
******************************************************/
#include <Heart.h>

// Create instance for sensor on analog input pin.
Heart heart(A1);

// Optional variables for lighting onboard LED on heartbeat
int counter;  // counter for timing routine without using delay
int LED = 13; // onboard LED
unsigned long litMillis = 0;        // will store how long LED was lit up
const long ledInterval = 50;        // interval at which to blink LED (milliseconds)

//variable for attenuating data flow to serial port prevents crashes
const long printInterval = 20;       // millis

boolean doOnce = true;    // for only performing actions once when heartbeat is detected

void setup() {
  Serial.begin(9600);  // works best in testing with 9600 or lower
  //optional LED for displaying heartbeat
  pinMode(LED, OUTPUT);
  litMillis = ledInterval;  // make sure the LED doesn't light up automatically
  
  // Initialize sensor.
  heart.reset();

  // uncomment below to redefine samplerate, default is 200Hz  
  //heart.setSampleRate(200); 
}

void loop() {
  // Update sensor.
  heart.update();
  unsigned long currentMillis = millis();    // update time
  if (currentMillis%printInterval == 0) {  //impose a delay to avoid taxing your serial port
  
    // Print-out different information.  
    Serial.print(heart.getNormalized()); // ADC values are normalized and mapped as float from 0.0 to 1.0
                                         // Note that if signal amplitude changes drastically the beat detection may
                                         // pause while the normalization process recalibrates
                                         
    Serial.print("\t");                  // tab separated values
    
    Serial.print(heart.getBPM());  
    Serial.print("\t");
    
    Serial.print(heart.bpmChange());     // maps changes in bpm and outputs as float from 0.0 to 1.0 
                                        // 0.5 is avg, < 0.5 as below average, > 0.5 above average.
    Serial.print("\t");
    Serial.println(heart.amplitudeChange()); // maps changes in signal amplitude and outputs as float from 0.0 to 1.0 
                                          // 0.5 is avg, < 0.5 as below average, > 0.5 above average.
  }                                   
  // An example of how to do something when a heartbeat is detected.
  // Remember that you should avoid using delays in order to preserve samplerate.
  
  if (heart.beatDetected()){  
    digitalWrite(LED, HIGH);
    if (doOnce == true){          // only perform these actions once when a heartbeat is detected
      litMillis = currentMillis;
      digitalWrite(LED, HIGH);    // turn on an LED for visual feedback that heartbeat occurred
      doOnce = false;             
    }
  } 
  else {
    doOnce = true;                // reset
  }

  // check to see if it's time to turn off the LED

  if (currentMillis - litMillis >= ledInterval) {   // if led interval has been surpassed turn it off
    digitalWrite(LED, LOW);
  }

}

