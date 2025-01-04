#include <Wire.h>
#include "Haptic_Driver.h"

Haptic_Driver hapDrive;

void setup() {
  Wire.begin();
  Serial.begin(115200);

  if (!hapDrive.begin()) {
    Serial.println("Haptic Driver initialization failed!");
    while (1);
  }

  Serial.println("Haptic Driver initialized!");

  hapticSettings motorSettings;
  motorSettings.motorType = LRA_TYPE;
  motorSettings.nomVolt = 1.4;
  motorSettings.absVolt = 1.45;
  motorSettings.currMax = 213;     
  motorSettings.impedance = 8.0;
  motorSettings.lraFreq = 80;

  if (!hapDrive.setMotor(motorSettings)) {
    Serial.println("Failed to configure actuator.");
    while (1);
  }

  Serial.println("Custom settings applied successfully.");

  if (!hapDrive.setOperationMode(DRO_MODE)) {
    Serial.println("Failed to set operation mode.");
    while (1);
  }
  hapDrive.enableFreqTrack(true);
  hapDrive.enableAcceleration(true);
  hapDrive.enableRapidStop(true);
  hapDrive.setVibrate(50);
  Serial.println("Driver ready.");
}

void loop() {
  
  delay(1000);
  uint8_t irqEvent = hapDrive.getIrqEvent();
  if (irqEvent != HAPTIC_SUCCESS) {
    Serial.print("Fault detected: ");
    Serial.println(irqEvent, HEX);
    hapDrive.clearIrq(irqEvent);
  }
}
