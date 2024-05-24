#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

void printf_init(void);
void usart_init();
char usart_getchar( void );
void usart_putchar( char data );
void usart_pstr (char *s);
unsigned char usart_kbhit(void);
int usart_putchar_printf(char var, FILE *stream);