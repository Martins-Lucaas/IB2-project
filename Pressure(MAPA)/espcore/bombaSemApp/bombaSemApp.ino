#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Configuração do sensor de pressão arterial
const int pino_sensor_pressao = 0;
const int janela_media_movel = 10;
const int janela_savitzky_golay = 7;
const int ganho = 23;
const float sensibilidade = 1.875;

// Pinos para controle da bomba e válvula
const int bomba_pos = 7;
const int bomba_neg = 8;
const int valvula_pos = 9;
const int valvula_neg = 10;

// Pressões de referência
const float pressao_alvo = 240.0;

// Variáveis para detecção de pressão sistólica e diastólica
float linha_base = 0;
float sistolica = 0;
float diastolica = 0;
bool sistolica_detectada = false;
bool diastolica_detectada = false;
bool inflando = true;

// Buffers e variáveis de aquisição de dados
float mediaMovelBuffer[10] = {0};
int mediaMovelIndex = 0;
float savitzkyGolayBuffer[7] = {0};
int savitzkyGolayIndex = 0;
bool updatingData = false;
unsigned long lastSampleTime = 0;
unsigned long acquisitionRate = 50;

// Tarefa FreeRTOS para aquisição de dados
TaskHandle_t vADCTaskHandle = NULL;

// Função para ler o valor do sensor de pressão
float readPressureSensor() {
  int valor_bruto = analogRead(pino_sensor_pressao);
  float tensao = (valor_bruto / 4095.0) * 3.3;
  float tensao_sensor = tensao / ganho;
  float pressao_kpa = tensao_sensor / (sensibilidade / 1000.0);
  return pressao_kpa * 7.50062; // Converte para mmHg
}

// Filtro de Média Móvel
float filtroMediaMovel(float novo_valor) {
  mediaMovelBuffer[mediaMovelIndex] = novo_valor;
  mediaMovelIndex = (mediaMovelIndex + 1) % janela_media_movel;

  float soma = 0;
  for (int i = 0; i < janela_media_movel; i++) {
    soma += mediaMovelBuffer[i];
  }
  return soma / janela_media_movel;
}

// Filtro Savitzky-Golay
float filtroSavitzkyGolay(float novo_valor) {
  savitzkyGolayBuffer[savitzkyGolayIndex] = novo_valor;
  savitzkyGolayIndex = (savitzkyGolayIndex + 1) % janela_savitzky_golay;

  const int coef[7] = {-3, 12, 17, 12, -3, -7, 2};
  float resultado = 0;
  for (int i = 0; i < janela_savitzky_golay; i++) {
    resultado += coef[i] * savitzkyGolayBuffer[(savitzkyGolayIndex + i) % janela_savitzky_golay];
  }
  return resultado / 35.0;
}

// Função para controlar a bomba
void controlarBomba(bool ligar) {
  if (ligar) {
    digitalWrite(bomba_pos, HIGH);
    digitalWrite(bomba_neg, LOW);
    Serial.println("Bomba ligada");
  } else {
    digitalWrite(bomba_pos, LOW);
    digitalWrite(bomba_neg, LOW);
    Serial.println("Bomba desligada");
  }
}

// Função para controlar a válvula
void controlarValvula(bool abrir) {
  if (abrir) {
    digitalWrite(valvula_pos, HIGH);
    digitalWrite(valvula_neg, LOW);
    Serial.println("Válvula aberta");
  } else {
    digitalWrite(valvula_pos, LOW);
    digitalWrite(valvula_neg, LOW);
    Serial.println("Válvula fechada");
  }
}

// Função para detectar pressão sistólica e diastólica
void detectar_pressao(float oscilacao, float linha_base) {
  float limiar_sistolica = 0.25 * oscilacao;
  float limiar_diastolica = 0.1 * oscilacao;

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

    // Desativa o processo de desinflação e mantém valores
    controlarValvula(false); // Fecha a válvula ao final da medição
    Serial.println("Medição completa. Aguarde nova ativação manual.");
    inflando = false;

  }
}

// Tarefa FreeRTOS para aquisição de dados do sensor de pressão arterial
void vADCTask(void *pvParameters) {
  while (1) {
    if (updatingData) {
      if (millis() - lastSampleTime >= acquisitionRate) {
        lastSampleTime = millis();
        float vADCvalue = readPressureSensor();
        Serial.print("vADC valor lido: ");
        Serial.println(vADCvalue);
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(bomba_pos, OUTPUT);
  pinMode(bomba_neg, OUTPUT);
  pinMode(valvula_pos, OUTPUT);
  pinMode(valvula_neg, OUTPUT);
  pinMode(pino_sensor_pressao, INPUT);

  xTaskCreate(vADCTask, "vADCTask", 4096, NULL, 1, &vADCTaskHandle);
}

void loop() {
  static unsigned long lastPressureSampleTime = 0;
  if (millis() - lastPressureSampleTime >= 200) {
    lastPressureSampleTime = millis();

    float pressao_atual = readPressureSensor();
    if (inflando) {
      if (pressao_atual >= pressao_alvo) {
        controlarBomba(false);
        controlarValvula(true);
        inflando = false;
        Serial.println("Pressão alvo alcançada - iniciando deflação");
      }
    } else {
      float pressao_linha_base = filtroMediaMovel(pressao_atual);
      float sinal_oscilacao = pressao_atual - pressao_linha_base;
      float oscilacao_filtrada = filtroSavitzkyGolay(sinal_oscilacao);
      detectar_pressao(oscilacao_filtrada, pressao_linha_base);
    }
  }
}
