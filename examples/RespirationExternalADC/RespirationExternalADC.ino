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

Arduino board's internal ADCs are usually limited to 10 bit values.
To get higher resolution in the breath signal, users might want to use an external ADC.
*/ 

#include "Respiration.h"

using namespace pq; // use Plaquette namespace to access Metro class

int LEDPin = 13; // LED pin
int thermistorPin = A0; // thermistor pin on the external ADC

Metro printerMetro (0.1); // print every 0.1 seconds

// Create instance for sensor 
 Respiration resp(unsigned long *getADCValue, 50, _16_BITS); 
 // Argument 1 : function to get external ADC value (will be defined below)
 // Argument 2 : sampling rate (here, 50 Hz)
 // Argument 3 : external ADC's resolution (format : _##_BITS)
 // In this example, we are using the ADS1115 and the ADS1X15 library by Rob Tillaart

unsigned long getADCValue(){

}

void setup() {
 // Initialize serial port
  Serial.begin(9600); 
  
  // Initialize sensor.
   resp.reset();
}

void loop() {
  //

  // Update sensor. 
   resp.update();
  
  if (printerMetro) { // print every 0.1 seconds
      // Get temperature 
      // (returns temperature in Celsius)
        Serial.print("Scaled: ");
        Serial.print(resp.getScaled());
        Serial.print(" ");

      // Exhaling ? Returns true if user is exhaling
        Serial.print("Exhaling: ");
        Serial.print(resp.isExhaling());
        Serial.print(" ");

      // Get amplitude level
      // (latest breath amplitudes are generally ===> 0 :  smaller than baseline, 0.5 : similar to baseline, 1 : larger than baseline)
        Serial.print("Amplitude level: ");
        Serial.print(resp.getAmplitudeLevel());
        Serial.print(" ");

      // Get RPM (respirations per minute) level 
      // (latest RPM values are generally ===> 0 :  smaller than baseline, 0.5 : similar to baseline, 1 : larger than baseline)
        Serial.print("RPM level: ");
        Serial.print(resp.getRpmLevel());   
        Serial.print(" ");

      Serial.println(" ");
  }    

  // An example of how to do something when user is exaling
  if(resp.isExhaling) { // if the user is exhaling
    digitalWrite(LEDPin, HIGH);
  } else{
    digitalWrite(LEDPin, LOW);
  }                               
}   
