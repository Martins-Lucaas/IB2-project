#include <WiFi.h>
#include <WebServer.h>
#include "esp_timer.h"

const int bomba_pos = 27;
const int bomba_neg = 26;
const int sole_pos = 14;
const int sole_neg = 12;
const int sensor = 34; 
const int ganho = 23;
const float sensibilidade = 1.875;

bool medirAtivo = false; // Variável para verificar se a medição está ativa
bool motorAtivo = false; // Variável para verificar se o motor deve estar ativo
WebServer server(80); // Cria o servidor web na porta 80
esp_timer_handle_t timer_handle; // Declaração da variável timer_handle

float pressao_kpa = 0.0; // Variável para armazenar o valor de pressão

void timer_callback(void* arg) {
  if (medirAtivo) { 
    int valor = analogRead(sensor);
    float tensao = (valor / 4095.0) * 3.3;
    float tensao_sensor = tensao / ganho;
    pressao_kpa = tensao_sensor / (sensibilidade / 1000.0); // Converte para pressão em kPa
  }
}

// Função para iniciar a medição e ligar o motor e a válvula
void iniciarMedicao() {
  medirAtivo = true;
  motorAtivo = true;
  Serial.println("Chamando iniciar Medicao...");  // Debug inicial

  // Ativa o motor e a válvula juntos
  digitalWrite(bomba_pos, HIGH);
  digitalWrite(bomba_neg, LOW);
  digitalWrite(sole_pos, HIGH);
  digitalWrite(sole_neg, LOW);

  server.sendHeader("Access-Control-Allow-Origin", "*"); 
  server.send(200, "text/plain", "Medição Iniciada");
  Serial.println("Medição iniciada.");
}

// Função para parar a medição e desligar o motor e a válvula
void pararMedicao() {
  medirAtivo = false;
  motorAtivo = false;
  Serial.println("Chamando parar Medicao...");  // Debug inicial

  // Desliga o motor e a válvula
  digitalWrite(bomba_pos, LOW);
  digitalWrite(bomba_neg, LOW);
  digitalWrite(sole_pos, LOW);
  digitalWrite(sole_neg, LOW);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Medição Parada");
  Serial.println("Medição parada.");
}

// Função para enviar o valor atual da pressão
void enviarValorPressao() {
  server.sendHeader("Access-Control-Allow-Origin", "*"); 
  server.send(200, "text/plain", String(pressao_kpa));
}

void setup() {
  Serial.begin(115200);

  // Configuração de pinos
  pinMode(bomba_pos, OUTPUT);
  pinMode(bomba_neg, OUTPUT);
  pinMode(sole_pos, OUTPUT);
  pinMode(sole_neg, OUTPUT);
  pinMode(sensor, INPUT);

  // Configuração do WiFi
  WiFi.begin("Martins Wifi6", "17031998"); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  
  // Exibe o IP do ESP32 quando conectado
  Serial.println("Conectado ao WiFi");
  Serial.print("Endereço IP do ESP32: ");
  Serial.println(WiFi.localIP());

  // Configuração do timer
  const esp_timer_create_args_t timer_args = {
    .callback = &timer_callback,
    .name = "frequency_timer"
  };
  esp_timer_create(&timer_args, &timer_handle);
  esp_timer_start_periodic(timer_handle, 400); // Dispara o callback a cada 400 ms

  // Configuração dos endpoints
  server.on("/startMeasurement", iniciarMedicao); // Endpoint para iniciar a medição
  server.on("/stopMeasurement", pararMedicao); // Endpoint para parar a medição
  server.on("/vADCvalue", enviarValorPressao); // Endpoint para enviar o valor de pressão
  server.begin();
}

void loop() {
  server.handleClient(); // Escuta as solicitações do servidor
}
