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
#include <BioData.h>

using namespace pq; // use Plaquette namespace to access Metro class

AnalogIn skinSensor(A6, INVERTED);

DigitalOut exhaleLed(13);

Wave sclSim(SINE, 120.0);
Wave scrSim(SINE, 2.0, 0.25);


// Create instance for sensor on analog input pin.
SkinConductance sc;

Metronome printerMetro(0.01); // print every 0.1 seconds

void setup() {
 // Setup Plaquette
  Plaquette.begin();

  sclSim.randomize(0.2);
  scrSim.randomize(0.7);
}

void loop() {
  // Call a Plaquette step at every loop
   Plaquette.step();

   float signal = sclSim + 0.1*scrSim;

  // Update sensor. 
  signal >> sc;
  
  // Print values
  // Prints out a few breath features extracted from signal. 
  // For list of all available features, see "Respiration.h"
  if (printerMetro) { // print every 0.1 seconds
    // Print-out values.
    beginPrintItems();
    printItem("raw", sc.getRaw());
    printItem("scaled", sc.getScaled());
    printItem("SCR", sc.getSCR());
    printItem("SCL", sc.getSCL());
    printItem("fast", sc.getFast());
    endPrintItems();
  }
}

bool firstPrint = false;
void beginPrintItems() {
  firstPrint = true;
  Serial.print(">");
}

void endPrintItems() {
  Serial.println();
}

void printItem(char* fieldName, float value) {
  if (firstPrint) {
    firstPrint = false;
  }
  else
    Serial.print(",");
  Serial.print(fieldName);
  Serial.print(":");
  Serial.print(value, 4);
}