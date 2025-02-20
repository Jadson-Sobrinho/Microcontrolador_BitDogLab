#include "hardware/spi.h"
#include "hardware/uart.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include <math.h>
#include "lib/animacao_0.h"
#include "lib/animacao_1.h"
#include "lib/animacao_2.h"
#include "lib/animacao_3.h"
#include "lib/animacao_4.h"
#include "lib/animacao_5.h"
#include "lib/animacao_6.h"
#include "lib/animacao_7.h"
#include "lib/animacao_8.h"
#include "lib/animacao_9.h"
#include "pio_matrix.pio.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "inc/ssd1306_font.h"
#include "hardware/i2c.h"
#include <ctype.h>
#include <string.h>

#define DEBOUNCE_TIME_MS 200  // Tempo mínimo entre leituras válidas
#define OUT_PINO 7

//botão de interupção
const uint button_0 = 5;
const uint button_1 = 6;

uint32_t last_press_time_0 = 0;  // Última ativação do botão_0
uint32_t last_press_time_1 = 0;  // Última ativação do botão_1



// Interrupção do botão para ativar/desativar animação
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time()); // Obtém o tempo atual em ms

    if (gpio == button_0)
    {
        if (current_time - last_press_time_0 < DEBOUNCE_TIME_MS) return; // Ignora se estiver dentro do tempo de debounce
        last_press_time_0 = current_time; // Atualiza o tempo do último acionamento
    } 
    else 
    {
        
       
    }

}

int main()
{
    PIO pio = pio0;
    uint32_t valor_led = 0;
    double r = 0.0, b = 0.0, g = 0.0;

    set_sys_clock_khz(128000, false);
    stdio_init_all();

    // Configurar interrupção no botão
    gpio_set_irq_enabled_with_callback(button_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(button_1, GPIO_IRQ_EDGE_FALL, true);

    //inicializar o botão de interrupção - GPIO5
    gpio_init(button_0);
    gpio_set_dir(button_0, GPIO_IN);
    gpio_pull_up(button_0);

    //inicializar o botão de interrupção - GPIO6
    gpio_init(button_1);
    gpio_set_dir(button_1, GPIO_IN);
    gpio_pull_up(button_1);

    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PINO);    

}
