// This example demonstrates gathering data relevant to respiration

// for more info see README at https://github.com/eringee/BioData/
/******************************************************
copyright Erin Gee 2017
Authors Erin Gee // Martin Peach // Thomas Ouellet-Fredericks // Luana Belinsky

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as published by
the Free Software Foundation.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    For more details: <http://www.gnu.org/licenses/>.
******************************************************/

/*
The sensor used here is a thermistor (temperature sensor), placed at airway entrance 
to obtain a breath temperature signal that is analysed to extract features.
*/ 

#include "Respiration.h"

using namespace pq; // use Plaquette namespace to access Metro class

int LEDPin = 13; // LED pin
int thermistorPin = 0; // thermistor pin

Metro printerMetro (0.1); // print every 0.1 seconds

// Create instance for sensor on analog input pin. 
//  Argument 1 : input pin 
//  Argument 2 (optional): sampling rate (default = 50 Hz)
 Respiration resp(thermistorPin);

void setup() {
 // Setup Plaquette
  Plaquette.begin();
 // Initialize serial port
  Serial.begin(9600); 
  
  // Initialize sensor.
   resp.reset();
}

void loop() {
  // Call a Plaquette step at every loop
   Plaquette.step();

  // Update sensor. 
   resp.update();
  
  // Print values
  // Prints out a few breath features extracted from signal. 
  // For list of all available features, see "Respiration.h"
  if (printerMetro) { // print every 0.1 seconds
      // Get raw ADC value
        Serial.print("Raw: ");
        Serial.print(resp.getRaw());
        Serial.print("  ");
      // Get scaled breath signal (returns a float between 0 and 1)
      // 0 = minimum value, inhale
      // 1 = maxmimum value, exhale
        Serial.print("Scaled: ");
        Serial.print(resp.getScaled());
        Serial.print("  ");

      // Exhaling ? Returns true if user is exhaling
        Serial.print("Exhaling:");
        Serial.print(resp.isExhaling());
        Serial.print("  ");

      // Get amplitude level
      // (latest breath amplitudes are generally ===> 0 :  smaller than baseline, 0.5 : similar to baseline, 1 : larger than baseline)
        Serial.print("Amplitude_level: ");
        Serial.print(resp.getAmplitudeLevel());
        Serial.print("  ");

      // Get RPM (respirations per minute) level 
      // (latest RPM values are generally ===> 0 :  smaller than baseline, 0.5 : similar to baseline, 1 : larger than baseline)
        Serial.print("RPM_level: ");
        Serial.print(resp.getRpmLevel());   
        Serial.print("  ");

      Serial.println(" ");
  }    

  // An example of how to do something when user is exaling
  if(resp.isExhaling) { // if the user is exhaling
    digitalWrite(LEDPin, HIGH);
  } else{
    digitalWrite(LEDPin, LOW);
  }                               
}   
