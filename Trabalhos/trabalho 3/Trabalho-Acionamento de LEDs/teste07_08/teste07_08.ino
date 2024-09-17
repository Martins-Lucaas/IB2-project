#include <Arduino.h>

#define verm_pin 18
#define infra_pin 19
#define leitura_pin 34

bool verm_estado = false;
bool infra_estado = false;

enum LedEstado { DESLIGADO, VERMELHO, INFRAVERMELHO };
LedEstado estado_atual = DESLIGADO;

const int janela_media_movel = 10;
float soma_vermelho = 0;
float soma_infravermelho = 0;
float media_vermelho = 0;
float media_infravermelho = 0;

float ac_vermelho = 0;
float ac_infravermelho = 0;
float r = 0;
float spo2 = 0;

int buffer_vermelho[janela_media_movel];
int buffer_infravermelho[janela_media_movel];
int indice = 0;

void piscarLEDs(void *parameter);
void lerAnalogico(void *parameter);
void calcularSpO2();

void setup() {
    Serial.begin(115200);
    pinMode(verm_pin, OUTPUT);
    pinMode(infra_pin, OUTPUT);
    digitalWrite(verm_pin, LOW);
    digitalWrite(infra_pin, LOW);

    for (int i = 0; i < janela_media_movel; i++) {
        buffer_vermelho[i] = 0;
        buffer_infravermelho[i] = 0;
    }

    xTaskCreate(piscarLEDs, "Piscar LEDs", 1024, NULL, 1, NULL);
    xTaskCreate(lerAnalogico, "Ler Analógico", 1024, NULL, 1, NULL);
}

void loop() {
}

void piscarLEDs(void *parameter) {
    while (1) {
        if (estado_atual == VERMELHO) {
            digitalWrite(verm_pin, LOW);
            digitalWrite(infra_pin, HIGH);
            estado_atual = INFRAVERMELHO;
        } else {
            digitalWrite(verm_pin, HIGH);
            digitalWrite(infra_pin, LOW);
            estado_atual = VERMELHO;
        }
        
        vTaskDelay(60 / portTICK_PERIOD_MS);
    }
}

void lerAnalogico(void *parameter) {
    while (1) {
        int leitura = analogRead(leitura_pin);

        if (estado_atual == VERMELHO) {
            soma_vermelho -= buffer_vermelho[indice];
            buffer_vermelho[indice] = leitura;
            soma_vermelho += buffer_vermelho[indice];
            media_vermelho = soma_vermelho / janela_media_movel;
            ac_vermelho = leitura - media_vermelho;

        } else if (estado_atual == INFRAVERMELHO) {
            soma_infravermelho -= buffer_infravermelho[indice];
            buffer_infravermelho[indice] = leitura;
            soma_infravermelho += buffer_infravermelho[indice];
            media_infravermelho = soma_infravermelho / janela_media_movel;
            ac_infravermelho = leitura - media_infravermelho;

            calcularSpO2();
        }

        indice = (indice + 1) % janela_media_movel;

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void calcularSpO2() {
    if (media_vermelho != 0 && media_infravermelho != 0) {
        r = (ac_vermelho / media_vermelho) / (ac_infravermelho / media_infravermelho);
        spo2 = 110 - 25 * r;
        spo2 = constrain(spo2, 0, 100);
        Serial.print("Razão R: ");
        Serial.print(r);
        Serial.print(" | SpO2: ");
        Serial.println(spo2);
    } else {
        Serial.println("Erro: Divisão por zero detectada ao calcular SpO2.");
    }
}
