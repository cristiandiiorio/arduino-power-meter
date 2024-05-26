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

  //ONLINE MODE
  uint16_t online_mode_time;
  online_mode_time = 1; // USER CHOOSES IT
  online_mode_time = 1000 * online_mode_time; // convert to ms

  //MAIN
  //simulating the sensor
  const uint8_t mask=(1<<6);
  DDRB &= ~mask;
  PORTB |= mask;
  uint16_t amp_count = 1;
  while(amp_count < 1000){
    int key=(PINB&mask)==0;

    if (key == 1){
      amp_value amp = {0, 0};
      amp.current = (float)rand() / RAND_MAX * 10.0;
      amp.timestamp = (online_mode_time / 1000) * amp_count;
      /*normal*/
      // UART_print_amp(amp);
      
      /*BINARY*/
      UART_print_amp_binary(&amp);

      amp_array[amp_count] = amp;
      amp_count++;
    }

    _delay_ms(online_mode_time);

  }
}
