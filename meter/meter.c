#include "meter.h"

void UART_print_amp(amp_value amp) {
  char output_str[30];
  sprintf(output_str, "at time %ds current is %dA\n", amp.timestamp, amp.current);
  UART_putString((uint8_t*)output_str);
}

void UART_print_amp_binary(amp_value *amp) {
  uint8_t* amp_ptr = (uint8_t*) amp;
  
  int i = 0;
  while (i < sizeof(amp_value)){
    UART_putChar(amp_ptr[i]);
    i++;
  }

}

int main(void){
  //INITIALIZATION ZONE
  UART_init();
  amp_value amp_array[ARRAY_SIZE];
  srand(time(0));

  uint16_t absolute_time = 1000;
  uint16_t amp_count = 0;

  //MAIN
  //simulating the sensor
  const uint8_t mask=(1<<6);
  DDRB &= ~mask;
  PORTB |= mask;

  _delay_ms(1000); // from delay.h, wait 1 sec

  while(amp_count < 5){
    int key=(PINB&mask)==0;

    if (key == 1){
      amp_value amp = {0, 0};
      amp.current = rand() % 1000;
      amp.timestamp = absolute_time/1000;
      /*normal*/
      // UART_print_amp(amp);
      
      /*BINARY*/
      UART_print_amp_binary(&amp);

      amp_array[amp_count] = amp;
      amp_count++;
    }

    _delay_ms(1000); // from delay.h, wait 1 sec
    absolute_time += 1000;
  }
}
