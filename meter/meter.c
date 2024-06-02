#include "meter.h"

volatile uint8_t timer_flag = 0;
volatile uint16_t amp_count = 0;

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
float adc_read(uint8_t ch) {
  // Select ADC channel ch must be 0-7
  ch &= 0b00000111;  // ANDing with 7 to make sure channel is between 0-7
  ADMUX = (ADMUX & 0xF8) | ch;  // Clearing the last three bits before ORing

  // Start single conversion
  ADCSRA |= (1 << ADSC);

  // Wait for conversion to complete
  while (ADCSRA & (1 << ADSC));

  // Combine the two 8-bit registers into a single 16-bit result
  return ADC; //466
}

ISR(TIMER5_COMPA_vect) {
  timer_flag = 1; // Set the flag to indicate timer overflow
  amp_count++;
}

int main(void){
  //INITIALIZATION ZONE
  UART_init();
  adc_init();
  amp_value amp_array[ARRAY_SIZE];

  //USER INPUT
  special_message sm = UART_read_special_message();

  //ONLINE MODE
  if(sm.mode=='o'){
    TCCR5A = 0;
    TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52) ; // set up timer with prescaler = 1024

    uint16_t ocrval = (uint16_t)(15.625 * 1000);
    OCR5A = ocrval;

    cli();
    TIMSK5 |= (1 << OCIE5A); // enable timer interrupt
    sei();
    while(1){
      if(timer_flag){
        timer_flag = 0;
        amp_value amp = {0, 0}; 
        amp.current = adc_read(0); //reading raw data from sensor
        amp.timestamp = (sm.payload) * amp_count;
        
        UART_send_amp_binary(&amp);
        amp_array[amp_count] = amp; //storing amp in array
      }
    }
  }
  //QUERY MODE
  else if(sm.mode=='q'){
    
  }
  //CLEARING MODE
  else if(sm.mode=='c'){
    memset(amp_array, 0, sizeof(amp_array));
  }
  
  /*
  float max_val;
  float new_val;
  float old_val = 0;
  float rms;
  float adjustment = 400;

  while (1) {
    new_val = adc_read(0);
    if (new_val > old_val) {
      old_val = new_val;
    } 
    else {
      _delay_us(50);
      new_val = adc_read(0);
      if (new_val < old_val) {
        max_val = old_val;
        old_val = 0;
      }
      rms = max_val * 5.00 * 0.707 / 1024;
      
      amp_value amp = {0, 0};
      amp.current = (rms * adjustment);
      amp.timestamp = 0;

      UART_send_amp_binary(&amp);
      
      _delay_ms(1000);
    }
  }
  */
}
