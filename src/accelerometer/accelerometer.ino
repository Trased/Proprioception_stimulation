#include <Wire.h>
#include <SPI.h>
#include <SFE_LSM9DS0.h>
#include <arduinoFFT.h>

#define LSM9DS0_XM   0x1D
#define LSM9DS0_G    0x6B
#define SAMPLE_RATE  800
#define NUM_SAMPLES  256
#define ACTUATOR_M   0.017f

float vReal[NUM_SAMPLES];
float vImag[NUM_SAMPLES];
ArduinoFFT<float> FFT(vReal, vImag, NUM_SAMPLES, SAMPLE_RATE);

LSM9DS0 imu(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

void setup() {
  Wire.begin();
  Wire.setClock(400000UL);

  Serial.begin(115200);
  delay(100);

  if (imu.begin() != 0x49D4) {
    Serial.println("Failed to initialize LSM9DS0!");
    while (1);
  }

  imu.setAccelODR(LSM9DS0::A_ODR_800);
  imu.setAccelScale(LSM9DS0::A_SCALE_2G);

  Serial.println("LSM9DS0 initialized at 400 kHz I²C.");
}

void loop() {
  if (!collectAccelerationData()) {
    Serial.println("Overrun—reduce SAMPLE_RATE or switch to SPI.");
    delay(200);
    return;
  }

  for (int i = 0; i < NUM_SAMPLES; i++) {
    vReal[i] *= 9.80665f;
  }

  float maxVal = vReal[0], minVal = vReal[0];
  for (int i = 1; i < NUM_SAMPLES; i++) {
    if (vReal[i] > maxVal) maxVal = vReal[i];
    if (vReal[i] < minVal) minVal = vReal[i];
  }
  float amplitude = (maxVal - minVal) * 0.5f;
  float forceAmp = ACTUATOR_M * amplitude;

  FFT.dcRemoval();
  FFT.windowing(FFT_WIN_TYP_WELCH, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  float peakFreq, peakAmp;
  FFT.majorPeak(&peakFreq, &peakAmp);

  Serial.print("Freq: ");
  Serial.print(peakFreq, 2);
  Serial.print(" Hz  |  Amp: ");
  Serial.print(amplitude, 6);
  Serial.print(" |  Force: ");
  Serial.print(forceAmp, 6);
  Serial.println(" N");

  delay(50);
}

bool collectAccelerationData() {
  const unsigned long samplingInterval = 1000000UL / SAMPLE_RATE;
  unsigned long startTime;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    startTime = micros();
    imu.readAccel();
    vReal[i] = imu.calcAccel(imu.ay);
    vImag[i] = 0.0f;

    while (micros() - startTime < samplingInterval) { }
    if (micros() - startTime > samplingInterval + 500) {
      return false;
    }
  }
  return true;
}
