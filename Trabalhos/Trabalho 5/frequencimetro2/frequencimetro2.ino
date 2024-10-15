#include "driver/rmt.h"
#include "esp_timer.h"

#define RMT_RX_CHANNEL RMT_CHANNEL_0  // Canal do RMT para recepção
#define SIGNAL_PIN GPIO_NUM_15                  // Pino onde o sinal está conectado
#define RMT_CLK_DIV 80                 // Divisor de clock (80MHz / 80 = 1MHz = 1us por tick)
#define MAX_PULSE_LEN 10000            // Máximo valor de duração do pulso (em ticks)

RingbufHandle_t rb = NULL;  // Ponteiro para a ring buffer

void setup() {
  Serial.begin(115200);

  // Configuração do RMT no modo receptor
  rmt_config_t rmt_rx;
  rmt_rx.rmt_mode = RMT_MODE_RX;         // Modo receptor
  rmt_rx.channel = RMT_RX_CHANNEL;       // Canal do RMT
  rmt_rx.gpio_num = SIGNAL_PIN;          // Pino GPIO de entrada
  rmt_rx.clk_div = RMT_CLK_DIV;          // Divisor de clock para 1us por tick
  rmt_rx.mem_block_num = 1;              // Usar 1 bloco de memória
  rmt_rx.rx_config.filter_en = true;     // Habilitar filtro de ruído
  rmt_rx.rx_config.filter_ticks_thresh = 100; // Ignorar pulsos menores que 100 ticks (100 us)
  rmt_rx.rx_config.idle_threshold = MAX_PULSE_LEN;  // Duração máxima do pulso antes de ser considerado inativo
  
  // Configurar e instalar o driver RMT
  rmt_config(&rmt_rx);
  rmt_driver_install(rmt_rx.channel, 1000, 0);
  
  // Obter a ring buffer associada ao canal RMT
  rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);

  // Iniciar a recepção de pulsos
  rmt_rx_start(RMT_RX_CHANNEL, true);
}

void loop() {
  // Receber os pulsos capturados da ring buffer
  size_t item_size;
  rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &item_size, pdMS_TO_TICKS(1000));

  if (item) {
    // Se houver um pulso capturado, calcular a frequência
    float pulse_duration_us = item->duration0;  // Duração do pulso em microssegundos

    if (pulse  _duration_us < 0) {
      float frequency = 1e6 / pulse_duration_us;  // Calcular a frequência em Hz
      Serial.print("Frequência: ");
      Serial.print(frequency);
      Serial.println(" Hz");
    }

    // Liberar a memória alocada para o item
    vRingbufferReturnItem(rb, (void*) item);
  }
}
