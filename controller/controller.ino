#include <Wire.h>
#include "Haptic_Driver.h"

Haptic_Driver hapDrive;

int event = 0; 

void setup(){

  Wire.begin();
  Serial.begin(115200);

  if( !hapDrive.begin())
    Serial.println("Could not communicate with Haptic Driver.");
  else
    Serial.println("Qwiic Haptic Driver DA7280 found!");

  if( !hapDrive.defaultMotor() ) 
    Serial.println("Could not set default settings.");

  hapDrive.enableFreqTrack(false);

  Serial.println("Setting I2C Operation.");
  hapDrive.setOperationMode(DRO_MODE);
  Serial.println("Ready.");

  delay(1000);

}

void loop(){
  hapDrive.setVibrate(25);
  delay(500); 
  hapDrive.setVibrate(0); 
  delay(500);

}
