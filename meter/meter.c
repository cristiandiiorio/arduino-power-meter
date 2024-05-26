#include "meter.h"

void UART_send_amp_binary(amp_value *amp) {
  uint8_t* amp_ptr = (uint8_t*) amp;
  int i = 0;
  while (i < sizeof(amp_value)){
    UART_putChar(amp_ptr[i]);
    i++;
  }

}

special_message UART_read_special_message() {
  special_message sm;
  uint8_t* sm_ptr = (uint8_t*) &sm;
  int i = 0;

  while(i < sizeof(special_message)){
    uint8_t c = UART_getChar();
    *sm_ptr = c;
    ++sm_ptr;
    ++i;
  }
  return sm;
}


int main(void){
  //INITIALIZATION ZONE
  UART_init();
  amp_value amp_array[ARRAY_SIZE];
  srand(time(0));
  
  //MAIN
  //simulating the sensor
  const uint8_t mask=(1<<6);
  DDRB &= ~mask;
  PORTB |= mask;
  uint16_t amp_count = 1;
  
  //ONLINE MODE
  uint16_t online_mode_time;
  online_mode_time = 1; // USER CHOOSES IT
  online_mode_time = 1000 * online_mode_time; // convert to ms

  _delay_ms(1000);
  
  // sm.mode== 'o' && sm.payload==1
  while(1){
    special_message sm = UART_read_special_message();
    amp_value amp = {0, 0};
    amp.current = 0;
    amp.timestamp = sm.payload;

    /*BINARY*/
    UART_send_amp_binary(&amp);
  
  }

  // while(amp_count < 1000){
  //   int key=(PINB&mask)==0;

  //   if (key == 1){
  //     amp_value amp = {0, 0};
  //     amp.current = (float)rand() / RAND_MAX * 10.0;
  //     amp.timestamp = (online_mode_time / 1000) * amp_count;

  //     /*BINARY*/
  //     UART_print_amp_binary(&amp);

  //     amp_array[amp_count] = amp;
  //     amp_count++;
  //   }

  //   _delay_ms(online_mode_time);

  // }
}
