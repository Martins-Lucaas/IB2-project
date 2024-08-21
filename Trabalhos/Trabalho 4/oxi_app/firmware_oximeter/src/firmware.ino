#include <WiFi.h>
#include <WebServer.h>

// Substitua com suas credenciais Wi-Fi
const char* ssid = "Seu_SSID";
const char* password = "Sua_Senha";

// Cria o servidor web na porta 80
WebServer server(80);

// Definição de pinos lidos no microcontrolador
#define verm_pin 12
#define infra_pin 13
#define leitura_pin 14

// Variáveis com o estado dos pinos, se estão ou não ativos
bool verm_estado = false;
bool infra_estado = false;
bool desl_estado = false;

// Variáveis
float infra_recebido, verm_recebido, desl_recebido; // Valores recebidos
float infra_vout, verm_vout, desl_vout; // Valores convertidos em tensão
int i = 0; // Índices para contagem
float verm_soma, infra_soma; // Soma dos valores dos vetores
float verm_media, infra_media; // Média dos vetores
float R; // Relação de resultados -> R = (Média dos resultados em vermelho) / (Média dos resultados em infra)
float spo2; // Resultado da saturação de oxigênio no sangue -> SpO2 = 104 - 28*R

const int intervalo = 1500; // Intervalo definido para quantidade de dados recebidos

// Vetores para salvar as variáveis
float verm_valores[intervalo], infra_valores[intervalo], desl_valores[intervalo];
float verm_subtracao[intervalo], infra_subtracao[intervalo];
float verm_filtrado[intervalo], infra_filtrado[intervalo];

void setup() {
  Serial.begin(115200);
  
  // Conectando ao Wi-Fi
  Serial.print("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Define rota para responder com o valor de SpO2
  server.on("/spo2", []() {
    realizar_leitura();
    filtro_modocomum();
    calculo_media();
    calculo_final();

    String spo2Str = String(spo2, 2);
    server.send(200, "text/plain", spo2Str);
  });

  // Inicia o servidor
  server.begin();

  // Configurando pinos do microcontrolador
  pinMode(verm_pin, OUTPUT);
  pinMode(infra_pin, OUTPUT);
  digitalWrite(verm_pin, LOW);
  digitalWrite(infra_pin, LOW);
}

// Loop principal
void loop() {
  server.handleClient();
}

void realizar_leitura() {
  // Quando o LED vermelho está ligado
  if (verm_estado) {
    for (i = 0; i < intervalo; i++) {
      verm_recebido = analogRead(leitura_pin);
      verm_vout = (verm_recebido * 3.3) / 4095;
      verm_valores[i] = verm_vout;
    }
    alternar_leds();
    i = 0;
  } else if (infra_estado) { // Quando o LED infravermelho está ligado
    for (i = 0; i < intervalo; i++) {
      infra_recebido = analogRead(leitura_pin);
      infra_vout = (infra_recebido * 3.3) / 4095;
      infra_valores[i] = infra_vout;
    }
    alternar_leds();
    i = 0;
  } else { // Quando os dois LEDs estão desligados
    for (i = 0; i < intervalo; i++) {
      desl_recebido = analogRead(leitura_pin);
      desl_vout = (desl_recebido * 3.3) / 4095;
      desl_valores[i] = desl_vout;
    }
    alternar_leds();
    i = 0;
  }
}

void filtro_modocomum() {
  for(int k = 0; k < intervalo; k++) {
    verm_subtracao[k] = verm_valores[k] - desl_valores[k];
    verm_filtrado[k] = abs(verm_subtracao[k]);
    infra_subtracao[k] = infra_valores[k] - desl_valores[k];
    infra_filtrado[k] = abs(infra_subtracao[k]);
  }
}

void calculo_media() {
  verm_soma = 0;
  infra_soma = 0;
  for (int i = 0; i < intervalo; i++) {
    verm_soma += verm_filtrado[i];
    infra_soma += infra_filtrado[i];
  }
  verm_media = verm_soma / intervalo;
  infra_media = infra_soma / intervalo;
}

void calculo_final() {
  R = verm_media / infra_media;
  spo2 = 104 - 28 * R;
}

void ligar_vermelho() {
  digitalWrite(verm_pin, HIGH);
  verm_estado = true;
  digitalWrite(infra_pin, LOW);
  infra_estado = false;
  desl_estado = false;
}

void ligar_infra() {
  digitalWrite(infra_pin, HIGH);
  infra_estado = true;
  digitalWrite(verm_pin, LOW);
  verm_estado = false;
  desl_estado = false;
}

void desligar_leds() {
  digitalWrite(infra_pin, LOW);
  infra_estado = false;
  digitalWrite(verm_pin, LOW);
  verm_estado = false;
  desl_estado = true;
}

void alternar_leds() {
  if (!verm_estado && !infra_estado) ligar_vermelho();
  else if (verm_estado && !infra_estado) ligar_infra();
  else desligar_leds();
}
