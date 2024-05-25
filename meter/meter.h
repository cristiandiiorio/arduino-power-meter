#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <string.h>
#include <time.h>
#include "my_uart.h" // this includes the UART_putString and initializes it

#define ARRAY_SIZE 1000

typedef struct __attribute__((packed)) amp_value {
  uint16_t current; 
  uint16_t timestamp;
} amp_value;


void UART_print_amp(amp_value amp);
void UART_print_amp_binary(amp_value *amp);

