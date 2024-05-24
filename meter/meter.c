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

amp_value amp_array[ARRAY_SIZE];

void UART_print_amp(amp_value amp) {
  char current_str[12];  // Buffer to hold the string representation of amp.current
  sprintf(current_str, "%dA\n", amp.current);  // Convert amp.current to string and append 'A' with a newline
  UART_putString((uint8_t*)current_str);  // Pass the string to UART_putString

  char timestamp_str[12];  // Buffer to hold the string representation of amp.timestamp
  sprintf(timestamp_str, "%ds\n", amp.timestamp);  // Convert amp.timestamp to string and append 's' with a newline
  UART_putString((uint8_t*)timestamp_str);  // Pass the string to UART_putString
}

int main(void){
  UART_init();

  srand(time(0));

  const uint8_t mask=(1<<6);
  DDRB &= ~mask;
  
  PORTB |= mask;
  
  uint16_t absolute_time = 0;
  uint16_t amp_count = 0;

  while(amp_count < 10){
    int key=(PINB&mask)==0;

    if (key == 1){
      amp_value amp;
      amp.current = rand();
      amp.timestamp = absolute_time/1000;
      UART_print_amp(amp);

      amp_array[amp_count] = amp;
      amp_count++;
    }

    _delay_ms(1000); // from delay.h, wait 1 sec
    absolute_time += 1000;
  }

  UART_putString((uint8_t*)"Printing last 10 measurements\n");

  for (int i = 0; i < amp_count; i++){
    UART_print_amp(amp_array[i]);
  }
}
