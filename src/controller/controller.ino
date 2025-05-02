#include <Wire.h>
#include "Haptic_Driver.h"

Haptic_Driver hapDrive;

// Test variables, will be modified at next tests
const int position = 1;
const int frequency = 80;
const int weight = 40;
const int vibrationValues[] = {0, 10, 20, 30, 40, 50, 60, 70};
const int delayValues[] = {100, 200, 300};
const int maxValues = 5;

void waitForReadySignal()
{
  while (true)
  {
    if (Serial.available() > 0)
    {
        break;
    }
  }
}

void sendToPC(int pos, int freq, int weight, String patternString)
{
  String data = String(pos) + ", " + String(freq) + ", " + String(weight) + ", " + "\"" + patternString + "\"";
  Serial.println(data);
}

String generateVibrationPattern()
{
  int numValues = random(1, maxValues + 1);
  
  int pattern[numValues];
  int delayPattern[numValues];
  
  for (int i = 0; i < numValues; i++)
  {
    pattern[i] = vibrationValues[random(0, 8)];
    delayPattern[i] = delayValues[random(0, 3)];
  }

  String patternString = "";
  for (int i = 0; i < numValues; i++)
  {
    patternString += String(delayPattern[i]) + "ms " + String(pattern[i]) + "%";
    if (i < numValues - 1)
    {
      patternString += ", ";
    }
  }

  for (int i = 0; i < numValues; i++)
  {
    hapDrive.setVibrate(pattern[i]);
    delay(delayPattern[i]);
  }
  hapDrive.setVibrate(0);

  return patternString;
}

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  if (!hapDrive.begin())
  {
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

  if (!hapDrive.setMotor(motorSettings))
  {
    Serial.println("Failed to configure actuator.");
    while (1);
  }

  Serial.println("Custom settings applied successfully.");

  if (!hapDrive.setOperationMode(DRO_MODE))
  {
    Serial.println("Failed to set operation mode.");
    while (1);
  }
  hapDrive.enableFreqTrack(false);
  hapDrive.enableAcceleration(false);
  hapDrive.enableRapidStop(false);


  Serial.println("Driver ready.");

  waitForReadySignal();
}

const int iterations = 10;
void loop()
{
  for(int i = 0; i < iterations; i ++)
  {
    String patternString = generateVibrationPattern();
    sendToPC(position, frequency, weight, patternString);

    // wait for next iteration
    delay(3000);

    uint8_t irqEvent = hapDrive.getIrqEvent();
    if (irqEvent != HAPTIC_SUCCESS)
    {
      hapDrive.clearIrq(irqEvent);
      hapDrive.setOperationMode(DRO_MODE);
    }
  }
  Serial.println("random data to end the pc script");
  Serial.flush();
  waitForReadySignal();
}
