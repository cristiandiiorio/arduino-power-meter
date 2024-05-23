#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <avr/io.h>
#include <time.h>
#include "../avr_common/uart.h" // this includes the printf and initializes it

#define ARRAY_SIZE 100000

typedef struct amp_value {
    int timestamp;
    int current;
} amp_value;


int main(void){
  // this initializes the printf/uart thingies
  printf_init(); 

  srand(time(0));

  // we connect the switch to pin 12
  // that is the bit 6 of port b
  
  const uint8_t mask=(1<<6);
  // we configure the pin as input, clearing the bit 6
  DDRB &= ~mask;
  
  // we enable pullup resistor on that pin
  PORTB |= mask;
  
  int absolute_time = 0;

  while(1){
    int key=(PINB&mask)==0; // we extract the bit value of the 6th bit

    if (key == 1){
      amp_value amp;
      amp.current = rand();
      amp.timestamp = absolute_time/1000;

      printf("current: %d, timestamp: %d\n", amp.current, amp.timestamp);

    }

    _delay_ms(1000); // from delay.h, wait 1 sec
    absolute_time += 1000;
  }
  
}
