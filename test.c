#include "meter.h"
#include <math.h>

volatile uint8_t uart_received_flag = 0;

volatile special_message sm;

void UART_send_amp_binary(amp_value *amp) {
  uint8_t* amp_ptr = (uint8_t*) amp;
  int i = 0;
  while (i < sizeof(amp_value)){
    UART_putChar(amp_ptr[i]);
    i++;
  }

}

special_message UART_read_special_message(void) {
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

// Function to read ADC channel 0
float adc_read(void) {
  ADMUX = (ADMUX & 0xF8) | 0;  

  // Start single conversion
  ADCSRA |= (1 << ADSC);

  // Wait for conversion to complete
  while (ADCSRA & (1 << ADSC));

  // Combine the two 8-bit registers into a single 16-bit result
  return ADC; 
}


float calculate_rms(float *buffer, uint16_t size) {
  float sum_of_squares = 0;
  for (uint16_t i = 0; i < size; i++) {
    sum_of_squares += buffer[i] * buffer[i];
  }
  float mean_of_squares = sum_of_squares / size;
  float rms = sqrt(mean_of_squares);
  return rms;
}

ISR(USART0_RX_vect) {
  uart_received_flag = 1; //set the flag to indicate that a message has been received
  sm = UART_read_special_message();
}

int main(void){
  //INITIALIZATION ZONE
  UART_init();
  adc_init();
  amp_value amp_array[ARRAY_SIZE];

  /*
  TCCR5A = 0;
  TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52) ; // set up timer with prescaler = 1024
  uint16_t ocrval = (uint16_t)(15.625 * 1000); //one measurement every second
  OCR5A = ocrval;

  cli();
  TIMSK5 |= (1 << OCIE5A); // enable timer interrupt
  sei();
  */
  sei();
  while(1){
    //------------------------------------------------------------------//
    //DETACHED MODE (NO RECEIVER CONNECTED)
    /*while(measurement_count < ARRAY_SIZE && !uart_received_flag){
      amp_value amp = {0, 0};
      amp.current = adc_read() ; // TODO:Calculate RMS value
      amp.timestamp = measurement_count;
      
      amp_array[measurement_count] = amp; // Storing amp in array
    }
    */
    //------------------------------------------------------------------//
    //USER MODE (RECEIVER CONNECTED)
    if(uart_received_flag){
      uart_received_flag = 0;
      
      //disable timer interrupt
      //TIMSK5 &= ~(1 << OCIE5A);

      //read message
      //special_message sm = UART_read_special_message();

      //ONLINE MODE
      if(sm.mode=='o'){
        //TODO: Cambiare timer
        /*
        TCCR5A = 0;
        TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52) ; // set up timer with prescaler = 1024
        const int time = sm.payload;
        uint16_t ocrval = (uint16_t)(15.625 * 1000 * time); //one measurement every x seconds
        OCR5A = ocrval;

        disable_interrupts();
        TIMSK5 |= (1 << OCIE5A); // enable timer interrupt
        enable_interrupts();
        */
        while(1){
          if(timer_flag){
            timer_flag = 0;
          
            amp_value amp = {0, 0};
            amp.current = adc_read() ; // TODO:Calculate RMS value
            amp.timestamp = measurement_count;

            UART_send_amp_binary(&amp);
          }
        }
      }
      //QUERY MODE TODO
      else if(sm.mode=='q'){
        amp_value amp = {9, 9};
        UART_send_amp_binary(&amp);
      }
      //CLEARING MODE
      else if(sm.mode=='c'){
        memset(amp_array, 0, sizeof(amp_array));
      }

    }
  }
}
