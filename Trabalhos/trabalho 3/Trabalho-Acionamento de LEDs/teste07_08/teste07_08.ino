#include <Arduino.h>

#define verm_pin 18
#define infra_pin 19
#define leitura_pin 34

bool verm_estado = false;
bool infra_estado = false;

enum LedEstado { DESLIGADO, VERMELHO, INFRAVERMELHO };
LedEstado estado_atual = DESLIGADO;

const int janela_media_movel = 100;
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
int leitura = 0;

void piscarLEDs(void *parameter);
void lerAnalogico(void *parameter);
void calcularSpO2();
void batimento();

unsigned long tempoAtual=0;
unsigned long  tempoAnterior=0;
int valorAtual=0;
int valorAnterior=0;
float derivada = 0; 
bool pico = false; 
int t1 = 0;
int t2 = 0;
int bpm = 0;


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
        leitura = analogRead(leitura_pin);
        tempoAtual= millis();

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
        batimento();
        vTaskDelay(20 / portTICK_PERIOD_MS);
        

        

    }
}

void calcularSpO2() {
  leitura = analogRead(leitura_pin);
    if (media_vermelho != 0 && media_infravermelho != 0 && leitura>2500 ) {
        r = (ac_vermelho / media_vermelho) / (ac_infravermelho / media_infravermelho);
        spo2 = 110 - 25 * r;
        spo2 = constrain(spo2, 0, 100);
        Serial.print("Razão R: ");
        Serial.print(r);
        Serial.print(" | SpO2: ");
        Serial.println(spo2);
        //Serial.println(leitura);
    }
    else if(leitura <= 2500){
      Serial.println("Coloque o dedo para calcular a saturação.");
    } 
     else {
        Serial.println("Erro: Divisão por zero detectada ao calcular SpO2.");
    }
}

void batimento(){
  tempoAtual= millis();
  
  int tempoAnterior = 0;
  
  int cont = 0;
  int tempo = 0;
  int t1 = 0;
  int t2 = 0;
  unsigned long deltaT = tempoAtual - tempoAnterior;
          if(deltaT > 0 ){
            valorAtual = leitura;
            Serial.println(leitura);
            Serial.println();
            derivada = (valorAtual - valorAnterior )/ deltaT;
            valorAnterior = valorAtual;
            tempoAnterior = tempoAtual;
            Serial.print("derivada: ");
            Serial.println(derivada);

            if (derivada <= 1){
              cont+=1;
              //tempo = tempo + tempoAtual;
              t1 = tempoAtual; 
              if (cont = 4){
                
                bpm = (2 * 60000)/tempo;
                Serial.print("bpm :");
                Serial.println(bpm);
                Serial.println();
                Serial.print("tempo :");
                Serial.println(tempo);
                Serial.println();

                cont = 0;
                tempoAtual = 0;
                valorAnterior = valorAtual; 
                tempo = 0;
              }
            


            }
          }

}