#define LED_RED_PIN 19
#define LED_IR_PIN 18


#define PHOTODIODE_PIN 26

#include "LowPassFilter.hpp"
volatile uint8_t ledState = 0;

void setup() {
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_IR_PIN, OUTPUT);

  pinMode(PHOTODIODE_PIN, INPUT);
  Serial.begin(9600);  // Inicialize a comunicação serial
  delay(2000); // Espera para a inicialização da comunicação serial
  
  xTaskCreatePinnedToCore(toggleLEDs, "Toggle LEDs", 1024, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(readPhotodiode, "Read Photodiode", 1024, NULL, 1, NULL, 1);
}

void loop() {
}

void toggleLEDs(void *parameter) {
  while (true) {
    switch (ledState) {
      case 0:
        digitalWrite(LED_RED_PIN, HIGH);
        digitalWrite(LED_IR_PIN, LOW);
        Serial.println("LED Vermelho Ligado");
        break;
      case 1:
        digitalWrite(LED_RED_PIN, LOW);
        digitalWrite(LED_IR_PIN, HIGH);
        Serial.println("LED IV Ligado");
        break;
      case 2:
        digitalWrite(LED_RED_PIN,LOW);
        digitalWrite(LED_IR_PIN,LOW);
        Serial.println("Ambos apagados");
      
     
    
    }
    ledState = (ledState + 1) % 3; // Alterna entre 0, 1, 2 e 3

    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void readPhotodiode(void *parameter) {
  while (true) {
    int sensorValue = analogRead(PHOTODIODE_PIN);
    Serial.println(sensorValue);
    
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}