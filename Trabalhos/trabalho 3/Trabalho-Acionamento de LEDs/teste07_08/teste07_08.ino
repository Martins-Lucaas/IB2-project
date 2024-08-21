#include <Arduino.h>
#define LED1_PIN 32
#define LED2_PIN 35
volatile uint8_t ledState = 0;
volatile int sensorValue = 1;
void setup() {
  Serial.begin(921600);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);


  xTaskCreatePinnedToCore(toggleLEDs,"Toggle LEDs", 1024, NULL, 1, NULL, 1);
}
void loop() {
}

void toggleLEDs(void *parameter) {
  while (true) {
    ledState = (ledState + 1) % 3;
    digitalWrite(LED1_PIN, (ledState == 0) ? HIGH : LOW);
    digitalWrite(LED2_PIN, (ledState == 2) ? LOW : HIGH);
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

