#include <Wire.h>
#include "MAX30105.h"
MAX30105 particleSensor; // inicializa MAX30102 com I2C
void setup()
{
  Serial.begin(115200);
  while(!Serial);// Devemos esperar que Teensy fique online
  delay(100);
  Serial.println("");
  Serial.println("MAX30102");
  Serial.println("");
  delay(100);
  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) // Use a porta I2C padrão, velocidade de 400 kHz
  {
    Serial.println("MAX30105 não foi encontrado. Verifique a ligação/alimentação.");
    while (1);
  }
  byte ledBrightness = 70; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}
void loop() {
  particleSensor.check(); //Check the sensor
  while (particleSensor.available()) {
      // ler IR armazenado
      Serial.print(particleSensor.getFIFOIR());
      Serial.print(",");
      // ler o vermelho armazenado
      Serial.println(particleSensor.getFIFORed());
      // ler o próximo conjunto de amostras
      particleSensor.nextSample();      
  }
}