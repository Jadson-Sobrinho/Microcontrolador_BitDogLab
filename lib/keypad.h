#ifndef KEYPAD_H
#define KEYPAD_H

#include "pico/stdlib.h"

// Definições de pinos e dimensões do teclado
#define ROWS 4
#define COLS 4

extern const uint gpio_rows[ROWS];
extern const uint gpio_cols[COLS];
extern char keys[ROWS][COLS];

void init_keypad();
char get_key();

#endif // KEYPAD_H