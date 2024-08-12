#include <Arduino.h>
#include "soc/gpio_reg.h" // Inclusão do cabeçalho dos registradores GPIO

// Definição dos pinos dos LEDs e do fotodiodo
#define LED1_PIN 14
#define LED2_PIN 12
#define PHOTODIODE_PIN 34

// Variáveis globais para manipulação dos LEDs
volatile uint8_t ledState = 0;

void setup() {
    // Configuração dos pinos dos LEDs como saída usando registradores
    REG_WRITE(GPIO_ENABLE_W1TS_REG, (1 << LED1_PIN)); // Define LED1_PIN como saída
    REG_WRITE(GPIO_ENABLE_W1TS_REG, (1 << LED2_PIN)); // Define LED2_PIN como saída

    // Configuração do pino do fotodiodo como entrada usando registradores
    // No ESP32, os pinos de entrada são configurados automaticamente

    // Inicialização da comunicação serial para depuração
    Serial.begin(115200);

    // Criação das tarefas FreeRTOS
    xTaskCreatePinnedToCore(
        toggleLEDs,          // Função da tarefa
        "Toggle LEDs",       // Nome da tarefa
        1024,                // Tamanho da stack
        NULL,                // Parâmetro da tarefa
        1,                   // Prioridade da tarefa
        NULL,                // Handle da tarefa
        1                    // Core da CPU (0 ou 1)
    );

    xTaskCreatePinnedToCore(
        readPhotodiode,      // Função da tarefa
        "Read Photodiode",   // Nome da tarefa
        1024,                // Tamanho da stack
        NULL,                // Parâmetro da tarefa
        1,                   // Prioridade da tarefa
        NULL,                // Handle da tarefa
        1                    // Core da CPU (0 ou 1)
    );
}

void loop() {
    // O loop principal permanece vazio
}

void toggleLEDs(void *parameter) {
    while (true) {
        ledState = (ledState + 1) & 0x03; // Alterna entre 0, 1, 2 e 3

        // Controle dos LEDs utilizando bitwise e registradores
        if (ledState == 0 || ledState == 3) {
            REG_WRITE(GPIO_OUT_W1TS_REG, (1 << LED1_PIN)); // Liga LED1
        } else {
            REG_WRITE(GPIO_OUT_W1TC_REG, (1 << LED1_PIN)); // Desliga LED1
        }

        if (ledState == 2 || ledState == 3) {
            REG_WRITE(GPIO_OUT_W1TS_REG, (1 << LED2_PIN)); // Liga LED2
        } else {
            REG_WRITE(GPIO_OUT_W1TC_REG, (1 << LED2_PIN)); // Desliga LED2
        }

        // Pequena pausa para estabilizar o estado
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void readPhotodiode(void *parameter) {
    while (true) {
        int sensorValue = analogRead(PHOTODIODE_PIN);
        Serial.println(sensorValue);
        
        // Pequena pausa para a próxima leitura
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
