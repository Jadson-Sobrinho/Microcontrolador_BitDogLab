# Documentação do Código

## Visão Geral
Este código implementa um jogo de reflexos utilizando o Raspberry Pi Pico, um display OLED SSD1306, uma matriz de LEDs controlada via PIO e dois buzzers para indicação sonora. O objetivo do jogo é que dois jogadores pressionem um botão o mais rápido possível após o sinal de "GO" ser dado. Se um jogador pressionar o botão antes do sinal, ele perde automaticamente.

## Dependências
O código depende das seguintes bibliotecas e arquivos de cabeçalho:
- `pico/stdlib.h`: Funções padrão da Raspberry Pi Pico SDK.
- `hardware/pio.h`, `hardware/clocks.h`, `hardware/i2c.h`, `hardware/pwm.h`: Controle do PIO, clock, I2C e PWM.
- `pico/bootrom.h`: Funções relacionadas ao boot da Pico.
- `inc/ssd1306.h`, `inc/ssd1306_font.h`: Controle do display OLED SSD1306.
- `pio_matrix.pio.h`: Programa PIO para controle da matriz de LEDs.
- `lib/animacao_0.h`, `lib/animacao_1.h`, `lib/animacao_2.h`, `lib/animacao_3.h`: Frames de animações utilizadas na contagem regressiva.

## Definição de Constantes e Variáveis Globais
- `DEBOUNCE_TIME_MS`: Tempo mínimo entre leituras válidas dos botões.
- `OUT_PINO`: Pino de saída para a matriz de LEDs.
- `NUMERO_DE_LEDS`: Número de LEDs na matriz.
- `button_0`, `button_1`: Pinos dos botões de entrada dos jogadores.
- `buzzer_0`, `buzzer_1`: Pinos de controle dos buzzers.
- `I2C_SDA`, `I2C_SCL`: Pinos do barramento I2C para o display OLED.
- `display_update_flag`, `display_message_type`: Flags para controle de mensagens no display.
- `player1Won`, `player2Won`: Flags indicando o jogador vencedor.
- `isInCount`, `signal_active`, `game_over`: Controle do estado do jogo.

## Funções Principais
### `matrix_rgb(double r, double g, double b)`
Converte valores RGB para formato de 32 bits compatível com a matriz de LEDs.

### `desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm)`
Atualiza os LEDs da matriz com base em um desenho.

### `exibir_animacao(double* animacao[], int num_desenhos, uint32_t valor_led, PIO pio, uint sm)`
Exibe uma animação na matriz de LEDs, interrompendo caso o jogo termine.

### `executar_animacao(int animacao_idx, uint32_t valor_led, PIO pio, uint sm)`
Seleciona e executa uma animação com base no índice fornecido.

### `gpio_irq_handler(uint gpio, uint32_t events)`
Interrupção dos botões para determinar o vencedor ou penalizar o jogador que apertar cedo.

### `set_buzzer_frequency(uint buzzer_pin, uint frequency)`
Configura a frequência do buzzer via PWM.

### `buzzer_off(uint buzzer_pin)`
Desliga o buzzer desativando o PWM.

## Fluxo do Programa (`main`)
1. Inicializa o clock, PIO, botões, display OLED e buzzers.
2. Entra no loop principal do jogo:
   - Reinicia o estado do jogo.
   - Exibe uma contagem regressiva usando animação na matriz de LEDs.
   - Aciona o buzzer e sinaliza o "GO".
   - Aguarda a resposta dos jogadores.
   - Atualiza o display com o vencedor ou penaliza um jogador caso tenha disparado cedo.
   - Repete o processo após um pequeno delay.

## Considerações Finais
O jogo implementa um mecanismo simples de reflexo e pode ser expandido para incluir mais efeitos visuais e sonoros. A adição de um cronômetro para medir tempos de resposta pode ser um aprimoramento futuro.

