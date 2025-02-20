#include "lib/keypad.h"

// Pinos das linhas e colunas
const uint gpio_rows[ROWS] = {18, 19, 20, 4};
const uint gpio_cols[COLS] = {9, 8, 16, 17};

// Mapeamento do teclado matricial
// Cols e Rows alterados para funcionamento do teclado fisico na BitDogLab - Wilton
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void init_keypad() {
    // Configura linhas do teclado como saídas
    for (int i = 0; i < ROWS; i++) {
        gpio_init(gpio_rows[i]);
        gpio_set_dir(gpio_rows[i], GPIO_OUT);
        gpio_put(gpio_rows[i], 1); // Inicialmente HIGH
    }

    // Configura colunas do teclado como entradas com pull-up
    for (int i = 0; i < COLS; i++) {
        gpio_init(gpio_cols[i]);
        gpio_set_dir(gpio_cols[i], GPIO_IN);
        gpio_pull_up(gpio_cols[i]);
    }
}

char get_key() {
    for (int row = 0; row < ROWS; row++) {
        gpio_put(gpio_rows[row], 0); // Ativa a linha atual (LOW)

        for (int col = 0; col < COLS; col++) {
            if (!gpio_get(gpio_cols[col])) { // Verifica se a coluna está pressionada
                gpio_put(gpio_rows[row], 1); // Restaura a linha para HIGH
                return keys[row][col];    // Retorna a tecla pressionada
            }
        }

        gpio_put(gpio_rows[row], 1); // Restaura a linha para HIGH
    }

    return 0; // Nenhuma tecla pressionada
}