#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <avr/io.h>
#include <time.h>
#include "my_uart.h" // this includes the UART_putString and initializes it

#define ARRAY_SIZE 1000

typedef struct amp_value {
  int timestamp;
  int current; //TODO: change to float (non so perche non riesco a stamparli)
} amp_value;


void UART_print_amp(amp_value amp);