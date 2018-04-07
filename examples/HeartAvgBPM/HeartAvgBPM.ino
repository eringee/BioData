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

// Optional variables for lighting onboard LED on heartbeat
int ledCounter;     // counter for led routine without delays
int LED = 13;       // onboard LED

// variables for averaging BPM over several heartbeats
int bpmCounter = 0;      // counter for counting bpmArray position     
int bpmArray[100];   // the array that holds bpm values. Define as a large number you don't need to use them all.
int avgBPM;          // value for displaying average BPM over several heartbeats
int arraySize = 10;   //determine how many beats you will collect and average

void setup() {
  Serial.begin(9600);
  
  //optional LED for displaying heartbeat
  pinMode(LED, OUTPUT);
  
  // Initialize sensor.
  heart.reset();
}

void loop() {
  // Update sensor.
  heart.update();

  // Print-out different information.  
  
  if (heart.beatDetected()){  
    ledCounter = 0;                         // reset LED counter
    digitalWrite(LED, HIGH);                // turn on an LED for visual feedback that heartbeat occurred
    bpmArray[bpmCounter] = heart.getBPM();  // grab a BPM snapshot every time a heartbeat occurs
    bpmCounter++;                           // increment the BPMcounter value
  }

  if (bpmCounter == (arraySize-1)) {                    // if you have grabbed 10 heartbeats
    avgBPM = 0;                             // refresh totalBPM
    
    for (int x = 0; x <= (arraySize-1); x++) {          // add up all the values in the array
  //    totalBPM += bpmArray[x];
      avgBPM = avgBPM + bpmArray[x];
    }

    avgBPM = avgBPM/arraySize;                 // divide by amount of values processed in array
    
    Serial.print("Your averaged BPM is: ");
    Serial.println(avgBPM);
    delay(5000);                            // stop everything to read the message for 5 seconds
    bpmCounter = 0;                     //  reset bpmCounter
    avgBPM = 0;
  }
  
  //TODO: incorporate millis into this process so it's universal.
  
  if (ledCounter < 5) {   //  on Arduino this number is 5, on Teensy it's more like 500.  

    ledCounter++;          // increment the counter every time
    Serial.println("Beat!");
  }  
  else {
    digitalWrite(LED, LOW);
  }

}

