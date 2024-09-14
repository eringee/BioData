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
to obtain a temperature signal that is analysed to extract breath features.

Arduino board's internal ADCs are usually limited to 10 bit values.
To get higher resolution in the breath signal, users might want to use an external ADC.
In this example, we are using the ADS1115 (16 bit resolution) and Rob Tillaart's "ADS1X15" library (https://github.com/RobTillaart/ADS1X15)
Consult the ADS1115 datasheet for more information.
*/ 

#include "Respiration.h" // inclue BioData Respiration module
#include <ADS1X15.h> // include external ADC library

int LEDPin = 13; // LED pin
int thermistorPin = 0; // thermistor pin on the external ADC

using namespace pq; // use namespace pq to access Plaquette objects in Arduino IDE

Metro printerMetro (0.1); // print every 0.1 seconds

ADS1115 ADS(0x49); // External ADC address
int adcValue; // variable for ADC value

// Create instance for sensor 
//  Argument 1 : function to get external ADC value 
//  Argument 2 (optional): sampling rate (default = 50 Hz)
 Respiration resp(&getADCValue); 

// Function to get external ADC value 
// This function will get called everytime a Respiration object peforms a sample()
// This function must have no arguments and return an "int"
int getADCValue(){ 
 return adcValue;
} 
void setup() {
 // Setup Plaquette
  Plaquette.begin();

 // Initialize serial port
  Serial.begin(9600); 
  
 // Initialize sensor.
  resp.reset();

 // Setup external ADC
  Wire.begin();
  ADS.begin();
  ADS.setGain(1);      //  4.096V
  ADS.setDataRate(7);  //  0 = slow   4 = medium   7 = fast
  ADS.setMode(0);      //  continuous mode
  ADS.readADC(thermistorPin);      //  first read to trigger
}

void loop() {
  // Call a Plaquette step at every loop
   Plaquette.step();

  // Update sensor. 
   resp.update();

  // Read external ADC
   adcValue = ADS.getValue();

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
  if(resp.isExhaling()) { // if the user is exhaling
    digitalWrite(LEDPin, HIGH);
  } else{
    digitalWrite(LEDPin, LOW);
  }                               
}   
