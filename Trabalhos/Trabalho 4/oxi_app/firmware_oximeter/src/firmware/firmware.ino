#include <Wire.h>
#include "MAX30105.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Martins Wifi6";  // Substitua pelo seu SSID
const char* password = "17031998";  // Substitua pela sua senha

MAX30105 particleSensor;
WebServer server(80);  // Cria o servidor na porta 80

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);  // Conecta à rede WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");
  Serial.println(WiFi.localIP());  // Exibe o IP do ESP32

  // Inicializa o sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) {
    Serial.println("MAX30105 não foi encontrado. Verifique as conexões.");
    while (1);
  }

  // Configuração do sensor MAX30102
  byte ledBrightness = 255;
  byte sampleAverage = 1;
  byte ledMode = 2;
  int sampleRate = 400;
  int pulseWidth = 69;
  int adcRange = 16384;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  // Define o manipulador para a requisição de dados
  server.on("/dados", handleDataRequest);
  server.begin();  // Inicia o servidor
}

void loop() {
  server.handleClient();  // Atende às requisições do cliente
}

void handleDataRequest() {
  particleSensor.check();  // Verifica o sensor

  if (particleSensor.available()) {
    long irValue = particleSensor.getFIFOIR();
    long redValue = particleSensor.getFIFORed();
    particleSensor.nextSample();  // Lê o próximo conjunto de amostras

    String data = "IR:" + String(irValue) + ",Red:" + String(redValue);
    
    // Adicione o cabeçalho CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", data);  // Envia os dados como resposta
  } else {
    // Adicione o cabeçalho CORS também para respostas sem dados
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Nenhum dado disponível");
  }
}
