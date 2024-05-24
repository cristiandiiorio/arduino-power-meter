#include "meter.h"

void UART_print_amp(amp_value amp) {
  char output_str[30];
  sprintf(output_str, "at time %ds current is %dA\n", amp.timestamp, amp.current);
  UART_putString((uint8_t*)output_str);
}

int main(void){
  //INITIALIZATION ZONE
  UART_init();
  amp_value amp_array[ARRAY_SIZE];
  srand(time(0));

  //MAIN
  const uint8_t mask=(1<<6);
  DDRB &= ~mask;
  
  PORTB |= mask;
  
  uint16_t absolute_time = 0;
  uint16_t amp_count = 0;

  while(amp_count < 7){
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

  UART_putString((uint8_t*)"Printing last 7 measurements\n");

  for (int i = 0; i < amp_count; i++){
    UART_print_amp(amp_array[i]);
  }
}
