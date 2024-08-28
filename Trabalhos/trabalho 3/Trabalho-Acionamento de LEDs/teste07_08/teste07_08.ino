// Configurações Globais
#define verm_pin 19
#define infra_pin 2
#define leitura_pin 33

// Variáveis com o estado dos pinos, se estão ou não ativos
bool verm_estado = false;
bool infra_estado = false;

// Variáveis para os valores recebidos
float infra_recebido, verm_recebido;

// Índices para contagem e resultados
float verm_soma, infra_soma; // Soma dos valores dos vetores
float verm_media, infra_media;
float R; // Relação entre os resultados das médias
float spo2; // Saturação de oxigênio no sangue

const int intervalo = 500; // Intervalo ajustado para quantidade de dados recebidos (ajuste conforme necessário)

// Vetores para salvar as variáveis
float verm_valores[intervalo], desl_valores[intervalo];
float infra_valores[intervalo];
float verm_subtracao[intervalo], infra_subtracao[intervalo];
float verm_filtrado[intervalo], infra_filtrado[intervalo];

// Variáveis para controle de tempo
unsigned long tempo_anterior_leitura = 0; // Tempo da última leitura
unsigned long tempo_anterior_calculo = 0; // Tempo do último cálculo de SpO2 e BPM
const unsigned long intervalo_leitura = 20; // Intervalo em milissegundos entre leituras (20 ms)
const unsigned long intervalo_calculo = 1000; // Intervalo em milissegundos para cálculo de SpO2 e BPM (1 segundo)

// Variáveis para cálculo de BPM
int contador_picos = 0;
unsigned long tempo_ultimo_pico = 0;
float bpm = 0;

// Configuração inicial do microcontrolador e configuração dos pinos
void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando sem BLE!");

    // Configurando pinos do microcontrolador
    pinMode(verm_pin, OUTPUT);
    pinMode(infra_pin, OUTPUT);

    // Deixando a saída dos pinos no modo desligado (LOW)
    configurarLEDs(false, false); // Ambos LEDs desligados
}

// Loop principal
void loop() {
    unsigned long tempo_atual = millis();

    // Verifica se o tempo desde a última leitura é maior que o intervalo desejado
    if (tempo_atual - tempo_anterior_leitura >= intervalo_leitura) {
        tempo_anterior_leitura = tempo_atual; // Atualiza o tempo da última leitura

        // Realizar leitura dos LEDs
        realizar_leitura();
        alternar_leds(); // Alterna o estado dos LEDs
    }

    // Verifica se é hora de realizar os cálculos de SpO2 e BPM
    if (tempo_atual - tempo_anterior_calculo >= intervalo_calculo) {
        tempo_anterior_calculo = tempo_atual; // Atualiza o tempo do último cálculo

        // Chamada de funções para cálculo de SpO2 e BPM
        filtro_modocomum();
        calculo_media();
        calculo_final();
        calcular_bpm();

        // Plotagem no serial monitor dos resultados finais
        Serial.print("SpO2: ");
        Serial.println(spo2);
        Serial.print("BPM: ");
        Serial.println(bpm);
        Serial.println();
    }

    delay(1); // Pequeno delay para aliviar o processamento
}

// Função para realizar leitura, dependendo do estado dos LEDs
void realizar_leitura() {
    static int i = 0; // Índice estático para manter a posição entre chamadas

    if (verm_estado) { // Quando o LED vermelho está ligado
        verm_recebido = analogRead(leitura_pin);
        verm_valores[i] = verm_recebido;
        infra_valores[i] = 0;
    } else if (infra_estado) { // Quando o LED infravermelho está ligado
        infra_recebido = analogRead(leitura_pin);
        infra_valores[i] = infra_recebido;
        verm_valores[i] = 0;
    } else { // Quando os dois LEDs estão desligados
        desl_valores[i] = analogRead(leitura_pin);
    }

    i = (i + 1) % intervalo; // Incrementa o índice e reseta quando atinge o intervalo
}

// Função de filtro de modo comum com filtragem adicional de suavização
void filtro_modocomum() {
    for (int k = 0; k < intervalo; k++) {
        verm_subtracao[k] = verm_valores[k] - desl_valores[k];
        infra_subtracao[k] = infra_valores[k] - desl_valores[k];
        
        // Filtro passa-baixa simples para suavização
        verm_filtrado[k] = 0.9 * abs(verm_subtracao[k]) + 0.1 * verm_filtrado[k];
        infra_filtrado[k] = 0.9 * abs(infra_subtracao[k]) + 0.1 * infra_filtrado[k];
    }
}

// Função para calcular a média dos vetores
void calculo_media() {
    verm_soma = 0;
    infra_soma = 0;

    for (int i = 0; i < intervalo; i++) {
        verm_soma += verm_filtrado[i];
        infra_soma += infra_filtrado[i];
    }

    verm_media = verm_soma / intervalo;
    infra_media = infra_soma / intervalo;

    // Verifica as médias calculadas
    Serial.print("Vermelho Médio: ");
    Serial.println(verm_media);
    Serial.print("Infravermelho Médio: ");
    Serial.println(infra_media);
}

// Função para fazer o cálculo final usando a relação de Beer-Lambert
void calculo_final() {
    if (infra_media != 0) { // Evitar divisão por zero
        R = verm_media / infra_media;
        spo2 = 110 - 25 * R; // Ajuste conforme necessário
        if (spo2 < 70) spo2 = 70;
        if (spo2 > 100) spo2 = 100;
    } else {
        spo2 = 0; // Caso infra_media seja zero, SpO2 é indefinido
        Serial.println("Aviso: Média infravermelha zero, SpO2 indefinido.");
    }
}

// Função para calcular BPM com base nos picos do sinal
void calcular_bpm() {
    int picos_detectados = 0;
    unsigned long tempo_atual = millis();
    unsigned long tempo_entre_picos = 0;

    for (int j = 1; j < intervalo - 1; j++) {
        // Ajuste dinâmico do limiar de pico
        float limiar_pico = verm_media * 0.6; // Exemplo de ajuste baseado na média

        if (verm_filtrado[j] > verm_filtrado[j - 1] && verm_filtrado[j] > verm_filtrado[j + 1] && verm_filtrado[j] > limiar_pico) {
            tempo_entre_picos = tempo_atual - tempo_ultimo_pico;
            if (tempo_entre_picos > 300) { // Apenas considere picos que estejam separados por mais de 300ms
                picos_detectados++;
                tempo_ultimo_pico = tempo_atual;
            }
        }
    }

    if (picos_detectados > 0) {
        bpm = (picos_detectados * 983000.0) / (intervalo * intervalo_leitura);
        picos_detectados = 0; // Reset do contador após cálculo
    } else {
        bpm = 0; // Caso não sejam detectados picos
        Serial.println("Aviso: Nenhum pico detectado para BPM.");
    }
}

// Função para configurar o estado dos LEDs
void configurarLEDs(bool estadoVermelho, bool estadoInfra) {
    digitalWrite(verm_pin, estadoVermelho ? HIGH : LOW);
    digitalWrite(infra_pin, estadoInfra ? HIGH : LOW);
    verm_estado = estadoVermelho;
    infra_estado = estadoInfra;
}

// Função para alternar os estados de LEDs com intervalo mais longo para estabilidade
void alternar_leds() {
    static unsigned long ultimo_troca = 0;
    if (millis() - ultimo_troca > 100) { // Alterna a cada 100ms para estabilidade
        if (!verm_estado && !infra_estado) {
            configurarLEDs(true, false); // Liga vermelho
        } else if (verm_estado && !infra_estado) {
            configurarLEDs(false, true); // Liga infravermelho
        } else {
            configurarLEDs(false, false); // Desliga ambos
        }
        ultimo_troca = millis();
    }
}
