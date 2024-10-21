#include "driver/pcnt.h"
#include "esp_timer.h"

#define SIGNAL_PIN 15  // Pino de entrada para o sinal da balança
#define PCNT_UNIT PCNT_UNIT_0
#define PIN 13

esp_timer_handle_t timer_handle;
int16_t pulse_count = 0;

void timer_callback(void* arg) {
  pcnt_get_counter_value(PCNT_UNIT, &pulse_count);
  pcnt_counter_clear(PCNT_UNIT);
}

void setup() {
  Serial.begin(115200);

  pcnt_config_t pcnt_config;
  pcnt_config.pulse_gpio_num = SIGNAL_PIN;
  pcnt_config.ctrl_gpio_num = PIN;
  pcnt_config.channel = PCNT_CHANNEL_1;
  pcnt_config.unit = PCNT_UNIT;
  
  pcnt_config.pos_mode = PCNT_COUNT_INC;
  pcnt_config.neg_mode = PCNT_COUNT_DIS;
  pcnt_config.lctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.counter_h_lim = 100000;
  pcnt_config.counter_l_lim = 0;
  
  pcnt_unit_config(&pcnt_config);

  pcnt_counter_pause(PCNT_UNIT);
  pcnt_counter_clear(PCNT_UNIT);
  pcnt_counter_resume(PCNT_UNIT);

  const esp_timer_create_args_t timer_args = {
    .callback = &timer_callback,
    .name = "frequency_timer"
  };
  esp_timer_create(&timer_args, &timer_handle);
  esp_timer_start_periodic(timer_handle, 1000);
}
void loop() {
    // Calcula a frequência
  unsigned long long int frequency = pulse_count*1000 ;
  Serial.print("Frequência: ");
  Serial.print(frequency);
  Serial.println(" Hz");
}