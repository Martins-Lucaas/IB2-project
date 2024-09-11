#define LED_RED_PIN 18
#define LED_IR_PIN 19
#define PHOTODIODE_PIN 33

#include <WiFi.h>
#include <WebServer.h>
#include <math.h>  // Para calcular senóides

volatile float spo2 = 0;
volatile int bpm = 0;

const char* ssid = "Martins Wifi6";  // Substitua pelo seu SSID
const char* password = "17031998";   // Substitua pela sua senha

WebServer server(80);  // Cria o servidor na porta 80

void setup() {
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_IR_PIN, OUTPUT);
  pinMode(PHOTODIODE_PIN, INPUT);  // Configuramos a porta analógica como entrada

  Serial.begin(115200);  // Inicialize a comunicação serial

  // Conecta ao WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");
  Serial.println(WiFi.localIP());  // Exibe o IP do ESP32

  // Define o manipulador para a requisição de dados
  server.on("/dados", handleDataRequest);
  server.begin();  // Inicia o servidor
}

void loop() {
  server.handleClient();  // Atende às requisições do cliente
}

void handleDataRequest() {
  int x = analogRead(PHOTODIODE_PIN);
  float ir_signal = 2048 + 300 * sin(2 * 3.1415 * millis() / 1000.0) + x ;
  float red_signal = 2048 + 200 * sin(2 * 3.1415 * millis() / 1000.0 + 3.1415 / 4) + x;

  // Criar a string com os valores formatados (IR e Red separados por vírgula)
  String response = "IR:" + String(ir_signal) + ",Red:" + String(red_signal);

  // Adicionar o cabeçalho CORS
  server.sendHeader("Access-Control-Allow-Origin", "*");

  // Enviar a resposta HTTP para o aplicativo Flutter
  server.send(200, "text/plain", response);
}
