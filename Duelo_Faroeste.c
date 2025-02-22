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

// Botões de interrupção
const uint button_0 = 5;
const uint button_1 = 6;

uint32_t last_press_time_0 = 0;  
uint32_t last_press_time_1 = 0;  

// Display OLED
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Variáveis voláteis para sinalizar atualização do display (usadas na IRQ)
volatile bool display_update_flag = false;
volatile int display_message_type = 0; 

bool animacao_ativa = false; // Estado da animação

int animacao_atual = 3;  // Começa na animação 3
const int num_animacoes = 4; // Número total de animações disponíveis

bool player1Won = false;
bool player2Won = false;

// isInCount indica que está na fase de contagem regressiva (botões devem ser ignorados)
volatile bool isInCount = false;
// signal_active indica que o sinal de tiro (“GO”) já foi dado
volatile bool signal_active = false;
// game_over indica que um jogador já disparou (válido ou antecipado)
volatile bool game_over = false;

uint32_t valor_led = 0;
PIO pio = pio0;
uint sm;

// Função para gerar cores RGB para matriz de LEDs
uint32_t matrix_rgb(double r, double g, double b) {
    return ((int)(g * 255) << 24) | ((int)(r * 255) << 16) | ((int)(b * 255) << 8);
}

// Atualiza os LEDs da matriz
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm) {
    for (int i = 0; i < NUMERO_DE_LEDS; i++) {
        if (player1Won) {
            valor_led = matrix_rgb(desenho[24 - i], 0, 0);
        } else if (player2Won) {
            valor_led = matrix_rgb(0, 0, desenho[24 - i]);
        } else {
            valor_led = matrix_rgb(desenho[24 - i], desenho[24 - i], desenho[24 - i]);
        }
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Exibe animação caso ativada – interrompe se o jogo acabar
void exibir_animacao(double* animacao[], int num_desenhos, uint32_t valor_led, PIO pio, uint sm) {
    for (int i = 0; i < num_desenhos; i++) {
        if (game_over) break;  // Para a animação se um botão já tiver sido pressionado
        desenho_pio(animacao[i], valor_led, pio, sm);
        sleep_ms(100);
    }
}

double* animacao_0[] = {desenho1, desenho2, desenho3, desenho4};
int num_desenhos = sizeof(animacao_0) / sizeof(animacao_0[0]);

double* animacao_1[] = {frame_1_1, frame_1_2, frame_1_3, frame_1_4, frame_1_5, frame_1_6, frame_1_7};
int num_frames = sizeof(animacao_1) / sizeof(animacao_1[0]);

double* animacao_2[] = {frame0, frame1, frame2, frame3, frame4, frame5, frame6, frame7, frame8, frame9};
int num_desenhos2 = sizeof(animacao_2) / sizeof(animacao_2[0]);

double* animacao_3[] = {frame00, frame01, frame02, frame03, frame04, frame05, frame06, frame07, frame08, frame09};
int num_desenhos_3 = sizeof(animacao_3) / sizeof(animacao_3[0]);

double* animacao_4[] = {desenho1_1, desenho2_2, desenho3_3, desenho4_4, desenho5_5, desenho6_6, desenho7_7};
int num_desenhos_4 = sizeof(animacao_4) / sizeof(animacao_4[0]);

double* animacao_5[] = {quadro0, quadro1, quadro2, quadro3, quadro4, quadro5};
int num_desenhos_5 = sizeof(animacao_5) / sizeof(animacao_5[0]);

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
        case 4:
            exibir_animacao(animacao_4, num_desenhos_4, valor_led, pio, sm);
            break;
        case 5:
            exibir_animacao(animacao_5, num_desenhos_5, valor_led, pio, sm);
            break;          
        default:
            printf("Animação inválida\n");
    }
}

// Interrupção dos botões
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (gpio == button_0) {
        if (current_time - last_press_time_0 < DEBOUNCE_TIME_MS) return;
        last_press_time_0 = current_time;
       
        if (!signal_active && isInCount) {
            // Player 1 apertou cedo – ele perde; Player 2 vence
            animacao_atual = 4;
            display_message_type = 3;
            display_update_flag = true;
            player1Won = false;
            player2Won = true;
            game_over = true;
        } else {
            // Disparo correto: Player 1 vence
            animacao_atual = 5;
            display_message_type = 1;
            display_update_flag = true;
            player1Won = true;
            player2Won = false;
            game_over = true;
        }
    } 
    else if(gpio == button_1) {
        if (current_time - last_press_time_1 < DEBOUNCE_TIME_MS) return;
        last_press_time_1 = current_time;

        if (!signal_active && isInCount) {
            // Player 2 apertou cedo – ele perde; Player 1 vence
            animacao_atual = 5;
            display_message_type = 4;
            display_update_flag = true;
            player1Won = true;
            player2Won = false;
            game_over = true;
        } else {
            // Disparo correto: Player 2 vence
            animacao_atual = 4;
            display_message_type = 2;
            display_update_flag = true;
            player1Won = false;
            player2Won = true;
            game_over = true;
        }
    }
}

int main() {
    set_sys_clock_khz(128000, false);
    stdio_init_all();

    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PINO);

    // Inicializa os botões de interrupção
    gpio_init(button_0);
    gpio_set_dir(button_0, GPIO_IN);
    gpio_pull_up(button_0);

    gpio_init(button_1);
    gpio_set_dir(button_1, GPIO_IN);
    gpio_pull_up(button_1);

    // Configura interrupção para os botões
    gpio_set_irq_enabled_with_callback(button_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicialização do I2C para o display OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização completa do display SSD1306
    ssd1306_init();
    
    // Configura a área de renderização do display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    while (true) {
        // Reinicia o estado do jogo
        signal_active = false;
        game_over = false;
        player1Won = false;
        player2Won = false;
        animacao_atual = 3;
        isInCount = true;

        // Contagem regressiva com animação
        for (int i = 3; i >= 0; i--) {
            if (game_over) break;  // Interrompe se um disparo já foi efetuado
            animacao_ativa = true;
            executar_animacao(animacao_atual, valor_led, pio, sm);
            sleep_ms(1000);
            animacao_atual--;    
        }
        isInCount = false;
        signal_active = true;

        // Aguarda até que um jogador dispare (se ainda não foi registrado)
        while (!game_over) {
            sleep_ms(10);
        }

        // Atualiza o display com a mensagem correspondente
        if (display_update_flag) {
            memset(ssd, 0, ssd1306_buffer_length);
            if (display_message_type == 1) {
                ssd1306_draw_string(ssd, 5, 0, "Player 1 Venceu");
            } else if (display_message_type == 2) {
                ssd1306_draw_string(ssd, 5, 0, "Player 2 Venceu");
            } else if (display_message_type == 3) {
                ssd1306_draw_string(ssd, 5, 0, "P1 tiro cedo");
            } else if (display_message_type == 4) {
                ssd1306_draw_string(ssd, 5, 0, "P2 tiro cedo");
            }
            render_on_display(ssd, &frame_area);
            display_update_flag = false;
        }
    
        sleep_ms(100);  // Pausa breve antes de reiniciar o jogo
    }
    
    return 0;
}
