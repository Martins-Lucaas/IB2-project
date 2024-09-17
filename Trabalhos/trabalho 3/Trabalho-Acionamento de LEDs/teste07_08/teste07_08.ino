#include <Arduino.h>

// Configurações Globais
#define verm_pin 18
#define infra_pin 19
#define leitura_pin 34

// Variáveis para controle dos LEDs
bool verm_estado = false;
bool infra_estado = false;

// Variável para armazenar o estado atual do LED
enum LedEstado { DESLIGADO, VERMELHO, INFRAVERMELHO };
LedEstado estado_atual = DESLIGADO;

// Variáveis para cálculo das componentes AC e DC
const int janela_media_movel = 10; // Janela para média móvel
float soma_vermelho = 0;
float soma_infravermelho = 0;
float media_vermelho = 0;
float media_infravermelho = 0;

// Variáveis para cálculo da razão R e SpO2
float ac_vermelho = 0;
float ac_infravermelho = 0;
float r = 0;
float spo2 = 0;

// Vetores para armazenar os sinais
int buffer_vermelho[janela_media_movel];
int buffer_infravermelho[janela_media_movel];
int indice = 0;

// Funções das tarefas
void piscarLEDs(void *parameter);
void lerAnalogico(void *parameter);
void calcularSpO2();

void setup() {
    Serial.begin(115200);

    // Configurando os pinos do microcontrolador
    pinMode(verm_pin, OUTPUT);
    pinMode(infra_pin, OUTPUT);

    // Inicializando os LEDs no estado desligado
    digitalWrite(verm_pin, LOW);
    digitalWrite(infra_pin, LOW);

    // Inicializando o buffer
    for (int i = 0; i < janela_media_movel; i++) {
        buffer_vermelho[i] = 0;
        buffer_infravermelho[i] = 0;
    }

    // Criação das tarefas
    xTaskCreate(piscarLEDs, "Piscar LEDs", 1024, NULL, 1, NULL);
    xTaskCreate(lerAnalogico, "Ler Analógico", 1024, NULL, 1, NULL);
}

void loop() {
    // Não é necessário colocar nada no loop principal
}

// Tarefa para alternar o piscar dos LEDs
void piscarLEDs(void *parameter) {
    while (1) {
        if (estado_atual == VERMELHO) {
            digitalWrite(verm_pin, LOW);  // Desliga LED vermelho
            digitalWrite(infra_pin, HIGH); // Liga LED infravermelho
            estado_atual = INFRAVERMELHO;
        } else {
            digitalWrite(verm_pin, HIGH);  // Liga LED vermelho
            digitalWrite(infra_pin, LOW); // Desliga LED infravermelho
            estado_atual = VERMELHO;
        }
        
        // Aguarda 60 ms antes de alternar novamente
        vTaskDelay(60 / portTICK_PERIOD_MS);
    }
}

// Tarefa para ler a porta analógica
void lerAnalogico(void *parameter) {
    while (1) {
        // Realizar leitura da porta analógica
        int leitura = analogRead(leitura_pin);

        // Processar o sinal para separar AC e DC
        if (estado_atual == VERMELHO) {
            // Atualizar a média móvel para componente DC
            soma_vermelho -= buffer_vermelho[indice];
            buffer_vermelho[indice] = leitura;
            soma_vermelho += buffer_vermelho[indice];
            media_vermelho = soma_vermelho / janela_media_movel;

            // Componente AC é a diferença entre o sinal atual e a média móvel
            ac_vermelho = leitura - media_vermelho;

            // Imprimir as componentes AC e DC do vermelho
        } else if (estado_atual == INFRAVERMELHO) {
            // Atualizar a média móvel para componente DC
            soma_infravermelho -= buffer_infravermelho[indice];
            buffer_infravermelho[indice] = leitura;
            soma_infravermelho += buffer_infravermelho[indice];
            media_infravermelho = soma_infravermelho / janela_media_movel;

            // Componente AC é a diferença entre o sinal atual e a média móvel
            ac_infravermelho = leitura - media_infravermelho;

            // Imprimir as componentes AC e DC do infravermelho
            // Calcular a saturação de oxigênio no sangue
            calcularSpO2();
        }

        // Atualizar o índice para a média móvel
        indice = (indice + 1) % janela_media_movel;

        // Aguarda 20 ms antes de fazer uma nova leitura
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

// Função para calcular SpO2
void calcularSpO2() {
    // Evitar divisão por zero
    if (media_vermelho != 0 && media_infravermelho != 0) {
        // Calcular a razão R
        r = (ac_vermelho / media_vermelho) / (ac_infravermelho / media_infravermelho);
        
        // Estimar a SpO2 usando a fórmula empírica
        spo2 = 110 - 25 * r;
        
        // Limitar os valores da SpO2 entre 0 e 100
        spo2 = constrain(spo2, 0, 100);
        
        // Imprimir o valor de SpO2
        Serial.print("Razão R: ");
        Serial.print(r);
        Serial.print(" | SpO2: ");
        Serial.println(spo2);
    } else {
        Serial.println("Erro: Divisão por zero detectada ao calcular SpO2.");
    }
}
