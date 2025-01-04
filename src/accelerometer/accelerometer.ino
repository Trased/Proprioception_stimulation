#include <Wire.h>
#include <SPI.h>
#include <SFE_LSM9DS0.h>
#include <arduinoFFT.h>

#define LSM9DS0_XM  0x1D
#define LSM9DS0_G   0x6B
#define SAMPLE_RATE 800
#define NUM_SAMPLES 256

double vReal[NUM_SAMPLES];
double vImag[NUM_SAMPLES];

ArduinoFFT<double> FFT(vReal, vImag, NUM_SAMPLES, SAMPLE_RATE);

LSM9DS0 imu(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

void setup() {
  Serial.begin(115200);

  if (imu.begin() != 0x49D4) {
    Serial.println("Failed to initialize LSM9DS0!");
    while (1);
  }

  imu.setAccelODR(LSM9DS0::A_ODR_800);
  imu.setAccelScale(LSM9DS0::A_SCALE_2G);

  Serial.println("LSM9DS0 initialized.");
}

void loop() {
  if (!collectAccelerationData()) {
    Serial.println("Data collection failed!");
    return;
  }
  FFT.dcRemoval();
  FFT.windowing(FFT_WIN_TYP_WELCH, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();


  double frequency, magnitute;
  FFT.majorPeak(&frequency, &magnitute);

  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.print(" Hz, Amplitude: ");
  Serial.println(calculateAmplitude());
}

bool collectAccelerationData() {
  unsigned long samplingInterval = 1000000 / SAMPLE_RATE;
  unsigned long startTime;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    startTime = micros();

    imu.readAccel();
    vReal[i] = imu.calcAccel(imu.ay);
    vImag[i] = 0;

    while (micros() - startTime < samplingInterval) {}

    if (micros() - startTime > samplingInterval + 500) {
      return false;
    }
  }
  return true;
}

double calculateAmplitude() {
  double sumOfSquares = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sumOfSquares += vReal[i] * vReal[i];
  }
  return sqrt(sumOfSquares / NUM_SAMPLES);
}
