#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "my_uart.h" // this includes the UART_putString and initializes it

#define ARRAY_SIZE 1000

typedef struct __attribute__((packed)) amp_value {
  float current; 
  uint16_t timestamp;
} amp_value;

typedef struct __attribute__((packed)) special_message {
  uint16_t payload; 
  char mode;
} special_message;

void UART_send_amp_binary(amp_value *amp);

special_message UART_read_special_message();


