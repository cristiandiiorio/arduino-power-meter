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

// Function to initialize the ADC
void adc_init(void) {
  // Select Vref=AVcc
  ADMUX |= (1 << REFS0);
  ADMUX &= ~(1 << REFS1);

  // Set ADC prescaler to 128 for 16 MHz clock (125 kHz ADC clock)
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // Enable ADC
  ADCSRA |= (1 << ADEN);
}

// Function to read the ADC value from a given channel (0-7)
uint16_t adc_read(uint8_t ch) {
  // Select ADC channel ch must be 0-7
  ch &= 0b00000111;  // ANDing with 7 to make sure channel is between 0-7
  ADMUX = (ADMUX & 0xF8) | ch;  // Clearing the last three bits before ORing

  // Start single conversion
  ADCSRA |= (1 << ADSC);

  // Wait for conversion to complete
  while (ADCSRA & (1 << ADSC));

  // Combine the two 8-bit registers into a single 16-bit result
  return (ADC);
}

float my_trunc(float num){
  int sign = (num < 0) ? -1 : 1; 
  num = num * sign;              
  num = num * 100;               
  num = num + 0.5;               
  int temp = (int)num;           
  num = temp / 100.0;            
  return sign * num;             
}

int main(void){
  //INITIALIZATION ZONE
  UART_init();
  adc_init();
  amp_value amp_array[ARRAY_SIZE];
  srand(time(0));

  const uint8_t mask = (1<<6);
  DDRB &= ~mask;
  PORTB |= mask;
  uint16_t amp_count = 1;

  //USER INPUT
  // special_message sm = UART_read_special_message();

  //ONLINE MODE
  /*
  if(sm.mode=='o'){
    uint16_t online_mode_time = sm.payload;
    online_mode_time = 1000 * online_mode_time; // convert to ms
    while(amp_count < 1000){
      int key=(PINB&mask)==0;
      
      if (key == 1){
        amp_value amp = {0, 0};
        //simulating the sensor
        amp.current = (float)rand() / RAND_MAX * 10.0;
        amp.timestamp = (online_mode_time / 1000) * amp_count;
        
        UART_send_amp_binary(&amp);
        amp_array[amp_count] = amp;
        amp_count++;
      }
      for (uint16_t i = 0; i < online_mode_time; i++) {
        _delay_ms(1);
      }
    }
  }
  else if(sm.mode=='q'){
    //QUERY MODE
    
  }
  else if(sm.mode=='c'){
    //CLEARING MODE
    
  }
  */
  uint16_t online_mode_time = 1;
  online_mode_time = 10 * online_mode_time; // convert to ms
  
  while(1){
    float sum_of_squares = 0;
    amp_count = 0;
    while(amp_count < 1000){ 
      float adc_voltage = adc_read(0);
      float current = (adc_voltage-485.0) * (5.0-1024.0);
      sum_of_squares += current * current;

      amp_count++;
      _delay_us(50);
    }

    float mean_square = sum_of_squares / amp_count;
    float rms_current = sqrt(mean_square);
    
    amp_value amp = {0, 0};
    amp.current = rms_current;
    amp.timestamp = 0;

    UART_send_amp_binary(&amp);
    // amp_array[amp_count] = amp;
    

    _delay_ms(1000);

  }
  
}
