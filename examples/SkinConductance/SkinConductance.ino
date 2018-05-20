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
#include <SkinConductance.h>

// Create instance for sensor on analog input pin.
SkinConductance sc(A6);

//variable for attenuating data flow to serial port prevents crashes
const long printInterval = 50;       // millis

void setup() {
  Serial.begin(9600);  // works best in testing with 9600 or lower

  // Initialize sensor.
  sc.reset();
  // uncomment below to redefine samplerate, default is 100Hz  
  //sc.setSampleRate(100);  
}

void loop() {
  // Update sensor.
  sc.update();
  unsigned long currentMillis = millis();    // update time
  if (currentMillis%printInterval == 0) {  //to avoid crashing serial port
    // Print-out values.
    Serial.print(sc.getSCR()); // this number changes only when a large enough spike in skin conductivity occurs.
    Serial.print(" ");
    Serial.print(sc.getSCL()); // the averaged level of skin conductivity as measured by the ADC
    Serial.print(" ");
    Serial.print(sc.getRaw()); // raw ADC value.
    Serial.println();
  }
}
