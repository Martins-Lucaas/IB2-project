#include <Arduino.h>

#define LED1_PIN 14
#define LED2_PIN 12
#define PHOTODIODE_PIN 34

volatile uint8_t ledState = 0;
volatile int sensorValue = 1;

void setup() {
  Serial.begin(115200);
  
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(PHOTODIODE_PIN, INPUT);

  xTaskCreatePinnedToCore(
    toggleLEDs,         
    "Toggle LEDs",      
    1024,              
    NULL,              
    1,                  
    NULL,              
    1                   
  );

  xTaskCreatePinnedToCore(
    readPhotodiode,   
    "Read Photodiode",  
    1024,               
    NULL,               
    1,                 
    NULL,               
    1                  
  );
}

void loop() {
}

void toggleLEDs(void *parameter) {
  while (true) {
    ledState = (ledState + 1) % 3; 

    digitalWrite(LED1_PIN, (ledState == 0) ? HIGH : LOW);
    digitalWrite(LED2_PIN, (ledState == 2) ? LOW : HIGH);

    vTaskDelay(sensorValue / portTICK_PERIOD_MS);
  }
}

void readPhotodiode(void *parameter) {
  while (true) {
    sensorValue = analogRead(PHOTODIODE_PIN);
    Serial.println(sensorValue);


    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
