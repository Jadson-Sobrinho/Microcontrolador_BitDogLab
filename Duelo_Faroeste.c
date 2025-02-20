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

//Display OLED
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Variáveis voláteis para sinalizar atualização do display (usadas na IRQ)
volatile bool display_update_flag = false;
volatile int display_message_type = 0; 


// Interrupção do botão
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time()); // Obtém o tempo atual em ms

    if (gpio == button_0)
    {
        if (current_time - last_press_time_0 < DEBOUNCE_TIME_MS) return; // Ignora se estiver dentro do tempo de debounce
        last_press_time_0 = current_time; // Atualiza o tempo do último acionamento
        display_message_type = 1;           // Código para "Botão A pressionado"
        display_update_flag = true;
        
    } 
    else if(gpio == button_1)
    {
        if (current_time - last_press_time_0 < DEBOUNCE_TIME_MS) return; 
        last_press_time_0 = current_time; 
        display_message_type = 2;           // Código para "Botão B pressionado"
        display_update_flag = true;
        
       
    }

}

int main()
{
    PIO pio = pio0;

    set_sys_clock_khz(128000, false);
    stdio_init_all();

    // Configurar interrupção no botão
    gpio_set_irq_enabled_with_callback(button_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

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
    

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização completo do OLED SSD1306
    ssd1306_init();
    
    // Configura a área de renderização do display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    // Cria um buffer para o display e limpa o display
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    while (true) {
            //memset(ssd, 0, ssd1306_buffer_length);  // Limpa o display
            char msg[26];  // Buffer para a mensagem a ser exibida
            render_on_display(ssd, &frame_area);  // Atualiza o display
        
    
        // Se a flag de atualização do display estiver setada, atualiza o display com a mensagem
        if (display_update_flag) {
            memset(ssd, 0, ssd1306_buffer_length);
            
            if (display_message_type == 1) {
                ssd1306_draw_string(ssd, 5, 0, "Player 1 Venceu");
            } 
            else if (display_message_type == 2) {
                ssd1306_draw_string(ssd, 5, 0, "Player 2 Venceu");
            } 

            render_on_display(ssd, &frame_area);  // Atualiza o display
            display_update_flag = false; // Evita atualizações repetidas
        }
    }
}
