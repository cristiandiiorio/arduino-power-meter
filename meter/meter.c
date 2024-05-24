#include "meter.h"

void UART_print_amp(amp_value amp) {
  char output_str[30];
  sprintf(output_str, "at time %ds current is %dA\n", amp.timestamp, amp.current);
  UART_putString((uint8_t*)output_str);
}

//binary
void UART_send_amp(amp_value amp) {
  uint8_t* data = (uint8_t*)&amp;
  for (size_t i = 0; i < sizeof(amp); i++) {
    UART_putChar(data[i]);
  }
}

void UART_read_amp(amp_value* amp) {
  uint8_t* data = (uint8_t*)amp;
  for (size_t i = 0; i < sizeof(amp_value); i++) {
    data[i] = UART_getChar();
  }
}

int main(void){
  //INITIALIZATION ZONE
  UART_init();
  amp_value amp_array[ARRAY_SIZE];
  srand(time(0));

  uint16_t absolute_time = 0;
  uint16_t amp_count = 0;

  //MAIN
  //simulating the sensor
  const uint8_t mask=(1<<6);
  DDRB &= ~mask;
  PORTB |= mask;

  UART_putString((uint8_t*)"Starting\n");

  //simulating normal operations then an interrupt comes at 7th measurement  
  while(amp_count < 7){
    int key=(PINB&mask)==0;

    if (key == 1){
      amp_value amp;
      amp.current = rand();
      amp.timestamp = absolute_time/1000;
      //UART_print_amp(amp);
      UART_send_amp(amp);

      amp_array[amp_count] = amp;
      amp_count++;
    }

    _delay_ms(1000); // from delay.h, wait 1 sec
    absolute_time += 1000;
  }

  UART_putString((uint8_t*)"Printing last 7 measurements\n");

  for (int i = 0; i < amp_count; i++){
    //UART_print_amp(amp_array[i]);
    UART_send_amp(amp_array[i]);
  }
}
