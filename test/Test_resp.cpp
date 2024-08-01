#include <Arduino.h>             // Arduino core

//================LIBRARIES==================//
#include "PlaquetteLib.h" //https://sofapirate.github.io/Plaquette/index.html
#include <Wire.h>            // Used to establish serial communication on the I2C bus
#include "Respiration.h"

//================CONSTANTS==================//

Respiration resp(3);
//================OBJECTS==================//
  //Printer timer
  Metro printerMetro(0.2);

//================VARIABLES==================//
  //Temperature
  float temperatureMA100; 

  //ADC MA100
  int16_t adcMA100;

//==================BEGIN====================//
  void begin() {
   Serial.begin(9600);

   Wire.begin();
   Wire.setClock(400000);   

   resp.reset();

     }

//==================STEP====================//
    void step() {

  // ===========UPDATE===========//
    if (printerMetro)
      resp.update();
  
  // ===========PRINT===========//
    if (printerMetro){

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


      // =======FLOW RATE======= //
      //Flow rate surges
        Serial.print("MA100_FRS: ");
        Serial.print(resp.getFlowRateSurges());
        Serial.print(" "); 
      //Flow rate variability
        Serial.print("MA100_FRV: ");
        Serial.print(resp.getFlowRateVariability());
        Serial.print(" "); 
     
      Serial.println(" ");
  }
}
