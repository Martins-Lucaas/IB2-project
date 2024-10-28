#include <WiFi.h>
#include <WebServer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Configuração de WiFi para conexão com a rede
const char *ssid = "Martins Wifi6";
const char *password = "17031998";

// Configuração do sensor de pressão arterial
const int pino_sensor_pressao = 5;       // Pino conectado ao sensor de pressão arterial
const int janela_media_movel = 10;       // Tamanho da janela para o filtro de média móvel
const int janela_savitzky_golay = 7;     // Tamanho da janela para o filtro Savitzky-Golay
const int ganho = 23;                    // Ganho do amplificador de instrumentação
const float sensibilidade = 1.875;       // Sensibilidade do sensor de pressão em mV/kPa

// Pinos para controle da bomba e válvula
const int bomba_pos = 9;
const int bomba_neg = 10;
const int valvula_pos = 8;
const int valvula_neg = 7;

// Pressões de referência em mmHg para o sistema
const float pressao_alvo = 240.0;        // Pressão alvo para parar a inflação
const float pressao_minima = 80.0;       // Pressão mínima para encerrar a deflação

// Variáveis para detecção de pressão sistólica e diastólica
float linha_base = 0;                    // Pressão de linha base (média móvel)
float sistolica = 0;                     // Valor da pressão sistólica detectada
float diastolica = 0;                    // Valor da pressão diastólica detectada
bool sistolica_detectada = false;        // Indicador de detecção de pressão sistólica
bool diastolica_detectada = false;       // Indicador de detecção de pressão diastólica
bool inflando = true;                    // Flag de controle de inflação ou deflação

// Buffers para os filtros de média móvel e Savitzky-Golay
float mediaMovelBuffer[10] = {0};        // Buffer de média móvel
int mediaMovelIndex = 0;                 // Índice do buffer de média móvel

float savitzkyGolayBuffer[7] = {0};      // Buffer Savitzky-Golay
int savitzkyGolayIndex = 0;              // Índice do buffer Savitzky-Golay

// Configuração do servidor web
WebServer server(80);                    // Cria servidor na porta 80

// Buffer e variáveis para aquisição de dados do sensor
const int bufferSize = 100;
float vADCBuffer[bufferSize];            // Buffer circular para armazenar leituras do sensor
int bufferIndex = 0;                     // Índice atual do buffer
bool updatingData = false;               // Flag para indicar se os dados estão sendo atualizados
unsigned long lastSampleTime = 0;        // Tempo da última amostra coletada
unsigned long acquisitionRate = 50;      // Taxa de aquisição (em ms) para a coleta de dados do sensor
unsigned long taxa_amostragem = 200;     // Taxa de amostragem para controle de pressão no loop principal

// Tarefa FreeRTOS para a aquisição de dados
TaskHandle_t vADCTaskHandle = NULL;

// Função para ler o valor do sensor de pressão arterial
float readPressureSensor() {
  int valor_bruto = analogRead(pino_sensor_pressao);        // Leitura do valor bruto do ADC
  float tensao = (valor_bruto / 4095.0) * 3.3;              // Converte para tensão
  float tensao_sensor = tensao / ganho;                     // Ajusta a tensão pelo ganho do amplificador
  float pressao_kpa = tensao_sensor / (sensibilidade / 1000.0); // Converte para kPa
  return pressao_kpa * 7.50062;                             // Converte de kPa para mmHg
}

// Filtro de Média Móvel: calcula a média dos últimos valores para suavizar o sinal
float filtroMediaMovel(float novo_valor) {
  mediaMovelBuffer[mediaMovelIndex] = novo_valor;
  mediaMovelIndex = (mediaMovelIndex + 1) % janela_media_movel;

  // A média móvel simples calcula a média de um conjunto de valores (definido por janela_media_movel)
  // e é útil para reduzir a variação de ruídos no sinal, produzindo uma leitura mais estável
  float soma = 0;
  for (int i = 0; i < janela_media_movel; i++) {
    soma += mediaMovelBuffer[i];
  }
  return soma / janela_media_movel;
}

// Filtro Savitzky-Golay: aplica um filtro específico para suavização de sinais
float filtroSavitzkyGolay(float novo_valor) {
  savitzkyGolayBuffer[savitzkyGolayIndex] = novo_valor;
  savitzkyGolayIndex = (savitzkyGolayIndex + 1) % janela_savitzky_golay;

  // O filtro Savitzky-Golay é usado para suavizar o sinal, preservando as características importantes
  // como picos e vales. Ele faz uma média ponderada dos valores do buffer com coeficientes específicos.
  const int coef[7] = {-3, 12, 17, 12, -3, -7, 2}; // Coeficientes do filtro para janela de 7 pontos
  float resultado = 0;
  for (int i = 0; i < janela_savitzky_golay; i++) {
    // A operação aplica cada coeficiente aos valores do buffer, ajudando a suavizar o sinal,
    // mantendo suas características estruturais
    resultado += coef[i] * savitzkyGolayBuffer[(savitzkyGolayIndex + i) % janela_savitzky_golay];
  }
  return resultado / 35.0; // Normaliza o resultado pela soma dos coeficientes
}

// Função para controlar a bomba de inflação do manguito
void controlarBomba(bool ligar) {
  if (ligar) {
    digitalWrite(bomba_pos, HIGH);
    digitalWrite(bomba_neg, LOW);
  } else {
    digitalWrite(bomba_pos, LOW);
    digitalWrite(bomba_neg, LOW);
  }
}

// Função para controlar a válvula de deflação do manguito
void controlarValvula(bool abrir) {
  if (abrir) {
    digitalWrite(valvula_pos, HIGH);
    digitalWrite(valvula_neg, LOW);
  } else {
    digitalWrite(valvula_pos, LOW);
    digitalWrite(valvula_neg, LOW);
  }
}

// Função para detectar os valores de pressão sistólica e diastólica
void detectar_pressao(float oscilacao, float linha_base) {
  float limiar_sistolica = 0.25 * oscilacao;  // Limiar para detectar a sistólica
  float limiar_diastolica = 0.1 * oscilacao;  // Limiar para detectar a diastólica

  if (!sistolica_detectada && oscilacao > limiar_sistolica) {
    sistolica = linha_base;
    sistolica_detectada = true;
    Serial.print("Pressão Sistólica detectada: ");
    Serial.print(sistolica);
    Serial.println(" mmHg");
  }

  if (sistolica_detectada && !diastolica_detectada && oscilacao < limiar_diastolica) {
    diastolica = linha_base;
    diastolica_detectada = true;
    Serial.print("Pressão Diastólica detectada: ");
    Serial.print(diastolica);
    Serial.println(" mmHg");
  }

  // Reinicia o sistema para uma nova leitura ao detectar ambos os valores
  if (sistolica_detectada && diastolica_detectada) {
    sistolica_detectada = false;
    diastolica_detectada = false;
    inflando = true;
    Serial.println("Medição completa. Reiniciando...");
  }
}

// Tarefa FreeRTOS para aquisição de dados do sensor de pressão arterial
void vADCTask(void *pvParameters) {
  while (1) {
    if (updatingData) {
      if (millis() - lastSampleTime >= acquisitionRate) {
        lastSampleTime = millis();
        float vADCvalue = readPressureSensor();
        vADCBuffer[bufferIndex] = vADCvalue;
        bufferIndex = (bufferIndex + 1) % bufferSize;
      }
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// Configuração inicial do sistema
void setup() {
  pinMode(bomba_pos, OUTPUT);
  pinMode(bomba_neg, OUTPUT);
  pinMode(valvula_pos, OUTPUT);
  pinMode(valvula_neg, OUTPUT);
  pinMode(pino_sensor_pressao, INPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  Serial.println("Conectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    float value = readPressureSensor();
    server.send(200, "text/plain", String(value, 4));
  });

  server.begin();
  Serial.println("Servidor iniciado");

  xTaskCreate(vADCTask, "vADCTask", 2048, NULL, 1, &vADCTaskHandle);

  controlarBomba(true);      // Inicia inflando o manguito
  controlarValvula(false);   // Mantém a válvula fechada
}

// Loop principal para controle de pressão
void loop() {
  server.handleClient();    // Garante que o servidor Web trate as requisições

  static unsigned long lastPressureSampleTime = 0;
  if (millis() - lastPressureSampleTime >= taxa_amostragem) {
    lastPressureSampleTime = millis();

    float pressao_atual = readPressureSensor();
    if (inflando) {
      if (pressao_atual >= pressao_alvo) {
        controlarBomba(false);      // Desliga a bomba ao atingir a pressão alvo
        controlarValvula(true);     // Abre a válvula para iniciar a deflação
        inflando = false;
      }
    } else {
      float pressao_linha_base = filtroMediaMovel(pressao_atual); // Calcula a linha base
      float sinal_oscilacao = pressao_atual - pressao_linha_base;  // Calcula a oscilação
      float oscilacao_filtrada = filtroSavitzkyGolay(sinal_oscilacao); // Suaviza a oscilação
      detectar_pressao(oscilacao_filtrada, pressao_linha_base);    // Detecta sistólica/diastólica

      if (pressao_atual <= pressao_minima) {
        controlarValvula(false);    // Fecha a válvula ao atingir a pressão mínima
      }
    }
  }
}
