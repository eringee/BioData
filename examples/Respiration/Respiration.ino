// This example demonstrates gathering data relevant to respiration

//Please note that if you are using a large timing capacitor that your circuit
//will take a few seconds to "warm up"

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
#include <Respiration.h>

// Create instance for sensor on analog input pin.
 Respiration resp(2);

//variable for attenuating data flow to serial port prevents crashes
const long printInterval = 20;       // millis

void setup() {
  Serial.begin(9600);  // works best in testing with 9600 or lower
  
  // Initialize sensor.
   resp.reset();

}

void loop() {
  // Update sensor. 
   resp.update();
  
  unsigned long currentMillis = millis();    // update time

  if (currentMillis%printInterval == 0) {  //to avoid crashing serial port
  // Print-out different information.  

      // =======BASE SIGNAL======= //
      // Raw
        Serial.print("MA100_R: ");
        Serial.print(resp.getRaw());
        Serial.print(" ");
      // Temperature
        Serial.print("MA100_T: ");
        Serial.print(resp.getTemperature());
        Serial.print(" ");
      // Normalized
        Serial.print("MA100_N: ");
        Serial.print(resp.getNormalized());
        Serial.print(" ");
      //Scaled
        Serial.print("MA100_NS: ");
        Serial.print(resp.getScaled());
        Serial.print(" ");
      //State
        Serial.print("MA100_S: ");
        Serial.print(resp.getState());
        Serial.print(" ");

      // =======AMPLITUDE======= //
      //Amplitude 
        Serial.print("MA100_A: ");
        Serial.print(resp.getAmplitude());
        Serial.print(" "); 
      //Amplitude normalized
        Serial.print("MA100_AN: ");
        Serial.print(resp.getAmplitudeNormalized());
        Serial.print(" "); 
      //Amplitude change
        Serial.print("MA100_AC: ");
        Serial.print(resp.getAmplitudeChange());
        Serial.print(" "); 
      //Amplitude delta
        Serial.print("MA100_AD: ");
        Serial.print(resp.getAmplitudeDelta());
        Serial.print(" "); 
      //Amplitude regularity
        Serial.print("MA100_AV: ");
        Serial.print(resp.getAmplitudeVariability());
        Serial.print(" "); 

      // =======RPM======= //
      //Interval
        Serial.print("MA100_I: ");
        Serial.print(resp.getInterval());
        Serial.print(" ");
      //RPM
        Serial.print("MA100_R: ");
        Serial.print(resp.getRpm());
        Serial.print(" ");
      //RPM normalized
        Serial.print("MA100_RN: ");
        Serial.print(resp.getRpmNormalized());
        Serial.print(" ");
      //RPM change
        Serial.print("MA100_RC: ");
        Serial.print(resp.getRpmChange());
        Serial.print(" ");
      //RPM variability
        Serial.print("MA100_RV: ");
        Serial.print(resp.getRpmVariability());
        Serial.print(" ");


      // // =======FLOW RATE======= //
      // //Flow rate surges
      //   Serial.print("MA100_FRS: ");
      //   Serial.print(resp.getFlowRateSurges());
      //   Serial.print(" "); 
      // //Flow rate variability
      //   Serial.print("MA100_FRV: ");
      //   Serial.print(resp.getFlowRateVariability());
      //   Serial.print(" "); 
     
      Serial.println(" ");
  }                                            
  }   
}

