// Respiration sensing using an external ADS1115 16-bit ADC.
//
// The sensor is a thermistor (temperature-sensitive resistor) placed under
// the nostrils. It detects temperature changes caused by airflow during
// breathing and produces a respiration signal.
//
// An external ADC (ADS1115) is used instead of the microcontroller's
// built-in ADC to provide higher resolution (16-bit vs typically 10-bit),
// improving sensitivity to small temperature changes.
//
// The ADS1115 communicates with the microcontroller over I²C.
// Requires the ADS1X15 library by Rob Tillaart:
// https://github.com/RobTillaart/ADS1X15
//
// For more info see README at https://github.com/eringee/BioData/
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

#include "Respiration.h" // include BioData Respiration module
#include <ADS1X15.h> // include ADS1115 library

using namespace pq; // use namespace pq to access objects from Plaquette library (Metronome)

int LEDPin = 13; // LED pin
int thermistorPin = 0; // thermistor connected to ADS1115 channel A0

Metro printerMetro(0.1); // print every 0.1 seconds

ADS1115 ADS(0x48); // ADS1115 I²C default address 

// Create Respiration instance
// Argument (optional): sampling rate (default = 50 Hz)
Respiration resp;

void setup() {
  Plaquette.begin(); // initialize Plaquette
  Serial.begin(9600); // initialize serial communication
  resp.reset(); // initialize sensor

  // Initialize ADS1115
  Wire.begin();       // join I²C bus
  ADS.begin();        // initialize
  ADS.setGain(1);     // input range 4.096V
  ADS.setDataRate(7); // 0 = slow, 4 = medium, 7 = fast
  ADS.setMode(0);     // continuous mode
  ADS.readADC(thermistorPin); // first read to trigger continuous mode
}

void loop() {
  Plaquette.step(); // Call a Plaquette step at every loop

  // Read ADS1115 and pass value to resp
  resp.update(ADS.getValue());

  if (printerMetro) { // print every 0.1 seconds
    // Raw ADC value from thermistor
    Serial.print("Raw: ");
    Serial.print(resp.getRaw());
    Serial.print("  ");

    // Scaled breath signal (float between 0 and 1)
    // 0 = inhale, 1 = exhale
    Serial.print("Scaled: ");
    Serial.print(resp.getScaled());
    Serial.print("  ");

    // Returns true if user is exhaling
    Serial.print("Exhaling: ");
    Serial.print(resp.isExhaling());
    Serial.print("  ");

    // Amplitude level (float between 0 and 1)
    // 0 = smaller than baseline, 0.5 = similar, 1 = larger
    Serial.print("Amplitude_level: ");
    Serial.print(resp.getAmplitudeLevel());
    Serial.print("  ");

    // RPM level (float between 0 and 1)
    // 0 = slower than baseline, 0.5 = similar, 1 = faster
    Serial.print("RPM_level: ");
    Serial.print(resp.getRpmLevel());
    Serial.print("  ");

    Serial.println();
  }

  // Example: light LED when exhaling
  digitalWrite(LEDPin, resp.isExhaling() ? HIGH : LOW);
}