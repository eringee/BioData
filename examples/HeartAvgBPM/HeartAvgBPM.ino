// This example demonstrates beating an LED on heartbeat, and determining an average BPM over several heartbeats
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

// variables for lighting onboard LED on heartbeat without delays
int LED = 13;       // onboard LED
unsigned long litMillis = 0;        // will store how long LED was lit up
const long ledInterval = 50;        // interval at which to blink LED (milliseconds)

// if you do not receive a heartbeat value in over 5 seconds, flush the BPM array and start fresh
const long flushInterval = 2000;    //interval at which to refresh values stored in array

boolean doOnce = true;   // makes sure that if a heartbeat is found that information is gathered only once during cycle

// variables for averaging BPM over several heartbeats
int bpmCounter = 0;      // counter for counting bpmArray position     
int bpmArray[100];   // the array that holds bpm values. Define as a large number you don't need to use them all.
int totalBPM = 0;          // value for displaying average BPM over several heartbeats
int arraySize = 5;   //determine how many beats you will collect and average

void setup() {
  Serial.begin(9600);  // works best in testing with 9600 or lower
  //optional LED for displaying heartbeat
  pinMode(LED, OUTPUT);
  litMillis = ledInterval;  // make sure the LED doesn't light up automatically
  
  // Initialize sensor.
  heart.reset();
}

void loop() {
  // Update sensor.
  heart.update();
  // update time
  unsigned long currentMillis = millis();   
  
  if (heart.beatDetected()){  
    if (doOnce == true){
    litMillis = currentMillis;
    digitalWrite(LED, HIGH);                // turn on an LED for visual feedback that heartbeat occurred
    
    bpmArray[bpmCounter] = heart.getBPM();  // grab a BPM snapshot every time a heartbeat occurs
    bpmCounter++;                           // increment the BPMcounter value
    doOnce = false;
    }
  }
  else {
    doOnce = true;
  }

  if (bpmCounter == (arraySize)) {                    // if you have grabbed enough heartbeats to average                                      
    
    for (int x = 0; x <= (arraySize-1); x++) {          // add up all the values in the array
      totalBPM = totalBPM + bpmArray[x];
      Serial.println(bpmArray[x]);
    }

    int avgBPM = totalBPM/arraySize;                 // divide by amount of values processed in array
    
    Serial.print("Your average BPM over ");
    Serial.print(arraySize);
    Serial.print(" beats is ");
    Serial.println(avgBPM);
    bpmCounter = 0;                     //  reset bpmCounter
    totalBPM = 0;                       // refresh totalBPM
    avgBPM = 0;                        // refresh avgBPM
    delay(2000);
  }
  
  // check to see if it's time to turn off the LED

  if (currentMillis - litMillis >= ledInterval) {   // if led interval has been surpassed turn it off
    digitalWrite(LED, LOW);
  }

  if (currentMillis - litMillis >= flushInterval){  // if you haven't received a heartbeat in a while keep the array fresh
    bpmCounter = 0;
  }
}

