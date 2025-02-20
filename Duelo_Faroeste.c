#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "lib/animacao_0.h"
#include "lib/animacao_1.h"
#include "lib/animacao_2.h"
#include "lib/animacao_3.h"
#include "lib/animacao_4.h"
#include "lib/animacao_5.h"
#include "pio_matrix.pio.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "inc/ssd1306_font.h"
#include "hardware/i2c.h"
#include <string.h>

#define DEBOUNCE_TIME_MS 50  // Tempo mínimo entre leituras válidas
#define OUT_PINO 7
#define NUMERO_DE_LEDS 25

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


// Variáveis globais para controlar o estado do LED e o tempo
bool led_on = false;
unsigned long last_time = 0;
bool animacao_ativa = false; // Estado da animação

int animacao_atual = 3;  // Começa na animação 3
const int num_animacoes = 4; // Número total de animações disponíveis


// Função para gerar cores RGB para matriz de LEDs
uint32_t matrix_rgb(double r, double g, double b) {
    return ((int)(g * 255) << 24) | ((int)(r * 255) << 16) | ((int)(b * 255) << 8);
}

// Atualiza os LEDs da matriz
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm) {
    for (int i = 0; i < NUMERO_DE_LEDS; i++) {
        valor_led = matrix_rgb(desenho[24 - i], desenho[24 - i], desenho[24 - i]);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Exibe animação caso ativada
void exibir_animacao(double* animacao[], int num_desenhos, uint32_t valor_led, PIO pio, uint sm) {
    //if (!animacao_ativa) return; // Só roda se estiver ativada
    for (int i = 0; i < num_desenhos; i++) {
        desenho_pio(animacao[i], valor_led, pio, sm);
        sleep_ms(100);
    }
}


double* animacao_0[] = {desenho1, desenho2, desenho3, desenho4, desenho5, desenho6, desenho7, desenho8, desenho9};
int num_desenhos = sizeof(animacao_0) / sizeof(animacao_0[0]);

double* animacao_1[] = {frame_1_1, frame_1_2, frame_1_3, frame_1_4, frame_1_5, frame_1_6, frame_1_7};
int num_frames = sizeof(animacao_1) / sizeof(animacao_1[0]);

double* animacao_2[] = {frame0, frame1, frame2, frame3, frame4, frame5, frame6, frame7, frame8, frame9};
int num_desenhos2 = sizeof(animacao_2) / sizeof(animacao_2[0]);

double* animacao_3[] = {frame00, frame01, frame02, frame03, frame04, frame05, frame06, frame07, frame08, frame09};
int num_desenhos_3 = sizeof(animacao_3) / sizeof(animacao_3[0]);

void executar_animacao(int animacao_idx, uint32_t valor_led, PIO pio, uint sm) {
    switch (animacao_idx) {
        
        case 0:
            exibir_animacao(animacao_0, num_desenhos, valor_led, pio, sm);
            break;
        case 1:
            exibir_animacao(animacao_1, num_frames, valor_led, pio, sm);
            break;
        case 2:
            exibir_animacao(animacao_2, num_desenhos2, valor_led, pio, sm);
            break;
        case 3:
            exibir_animacao(animacao_3, num_desenhos_3, valor_led, pio, sm);
            break;             
        default:
            printf("Animação inválida\n");
    }
}

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
    uint32_t valor_led = 0;
    double r = 0.0, b = 0.0, g = 0.0;

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
        // Executa a animação antes de verificar se o display precisa ser atualizado
        for (int i = 4; i > 0; i--) {
            executar_animacao(animacao_atual, valor_led, pio, sm);
            sleep_ms(1000);
            animacao_atual--;
        }
    
        // Se a flag de atualização do display estiver setada, exibe a mensagem correspondente
        if (display_update_flag) {
            memset(ssd, 0, ssd1306_buffer_length); // Limpa o display apenas quando necessário
            
            if (display_message_type == 1) {
                ssd1306_draw_string(ssd, 5, 0, "Player 1 Venceu");
            } 
            else if (display_message_type == 2) {
                ssd1306_draw_string(ssd, 5, 0, "Player 2 Venceu");
            } 
    
            render_on_display(ssd, &frame_area);  // Atualiza o display apenas quando necessário
            display_update_flag = false; // Reseta a flag para evitar atualizações repetidas
        }
    
        sleep_ms(100);  // Pequena pausa para evitar alto consumo de CPU
    }
    
}
