#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <avr/io.h>
#include <time.h>
#include "../avr_common/uart.h" // this includes the printf and initializes it

#define ARRAY_SIZE 1000

typedef struct amp_value {
    int timestamp;
    int current; //TODO: change to float (non so perche non riesco a stamparli)
} amp_value;

amp_value amp_array[ARRAY_SIZE];

int main(void){
  printf_init(); 

  srand(time(0));

  const uint8_t mask=(1<<6);
  DDRB &= ~mask;
  
  PORTB |= mask;
  
  int absolute_time = 0;

  int amp_count = 0;

  while(amp_count < 5){
    int key=(PINB&mask)==0;

    if (key == 1){
      amp_value amp;
      amp.current = rand();
      amp.timestamp = absolute_time/1000;
      printf("current: %d, timestamp: %d\n", amp.current, amp.timestamp);
      
      amp_array[amp_count] = amp;
      amp_count++;
    }

    _delay_ms(1000); // from delay.h, wait 1 sec
    absolute_time += 1000;
  }

  printf("Printing last 5 measurements\n");

  for (int i = 0; i < amp_count; i++){
    printf("current: %d, timestamp: %d\n", amp_array[i].current, amp_array[i].timestamp);
  }
}
