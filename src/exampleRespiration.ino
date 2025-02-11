#include "Respiration.h"

int sampleRate = 50; 
int pin = A0;
int externalADCValue; 
float temperature; 


// Modes : READPIN, EXTERNALADC, MEASUREMENT
Respiration resp_readPin(READPIN, pin, sampleRate); 
Respiration resp_extADC(EXTERNALADC, &externalADCValue, sampleRate); 
Respiration resp_temp(MEASUREMENT, &temperature, sampleRate); 

void setup() {
// Initialize serial port
    Serial.begin(9600); 
    
// Initialize sensor.
    resp_readPin.reset();
    resp_extADC.reset();
    resp_temp.reset();
}


void loop() {   

// Read values if needed
externalADCValue = ADS.getValue(); 
temperature = MA100.getTemperature(); 

// Update sensor.
resp_adc10.update(); 
resp_adc16.update(); 
resp_temp.update();

// Get descriptors
// Raw signal
Serial.print(resp_readPin.getRaw()); 
Serial.print(" "); 
Serial.print(resp_extADC.getRaw()); 
Serial.print(" "); 
Serial.print(resp_temp.getRaw()); 
Serial.println();

Serial.print(resp_readPin.getScaled()); 
Serial.print(" "); 
Serial.print(resp_extADC.getScaled()); 
Serial.print(" "); 
Serial.print(resp_temp.getScaled()); 
Serial.println();

// etc
}

// PUARA
Respiration resp(READPIN, pin, sampleRate);

int respADC;
float respTemperature;

Amplitude respirationAmplitude_object(&resp);
Amplitude respirationAmplitude_adc(&respADC);
Amplitude respirationAmplitude_temp(&respTemperature);

void setup() {
// Initialize serial port
    Serial.begin(9600); 
    
// Initialize sensor.
    resp.reset();
}

void loop() {

// Read values if needed
respADC = analogRead(pin);
respTemperature = MA100.getTemperature(respADC);

resp.update();
respirationAmplitude.update();

Serial.println(respirationAmplitude_object.getRawAmplitude());
Serial.println(respirationAmplitude_adc.getRawAmplitude());
Serial.println(respirationAmplitude_temp.getRawAmplitude());

Serial.println(respirationAmplitude_object.getScaledAmplitude());
Serial.println(respirationAmplitude_adc.getScaledAmplitude()); //how would we scale? 
Serial.println(respirationAmplitude_temp.getScaledRawAmplitude());
}

// PUARA
Heart heart(heartPin);
SkinConductance gsr(gsrPin);
Respiration resp(respPin);

BioData biodata(&heart, &gsr, &resp);

void setup() {
// Initialize serial port
    Serial.begin(9600); 
    
// Initialize sensors
    heart.reset();
    gsr.reset();
    resp.reset();
}

void loop() {

// Update sensors
 heart.update();
 gsr.update();
 resp.update();

// Get descriptors
Serial.println(biodata.heart.getNormalized());
Serial.println(biodata.gsr.getSCL());
Serial.println(biodata.resp.getScaled());

}
