#include "meter.h"
#include <math.h>

#define BUFFER_SIZE 1000

volatile uint8_t timer_flag = 0;
volatile uint16_t measurement_count = 0;

volatile special_message received_message;
volatile uint8_t message_received = 0;
volatile uint8_t* sm_ptr;
volatile int sm_index = 0;

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

ISR(TIMER5_COMPA_vect) {
  timer_flag = 1; // Set the flag to indicate timer overflow
  measurement_count++;
}

ISR(USART0_RX_vect) {
  // Initialize pointer to the special message structure
  sm_ptr = (uint8_t*)&received_message;
  // Read the received byte
  uint8_t received_byte = UART_getChar();

  // Store the byte in the special_message structure
  *sm_ptr = received_byte;
  ++sm_ptr;
  ++sm_index;

  // Check if the entire message has been received
  if (sm_index >= sizeof(special_message)) {
    sm_index = 0;  // Reset index for the next message
    message_received = 1;  // Indicate that a message has been received
  }
}



int main(void){
  //INITIALIZATION ZONE
  UART_init();
  adc_init();
  amp_value amp_array[ARRAY_SIZE];

  //----------------------------------------------------//
  //DETACHED MODE (NO RECEIVER CONNECTED)


  //----------------------------------------------------//
  //USER MODE (RECEIVER CONNECTED)
  while(1){
    if(message_received){
      special_message sm = received_message;
      //ONLINE MODE
      if(sm.mode=='o'){
        TCCR5A = 0;
        TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52) ; // set up timer with prescaler = 1024
        const int time = sm.payload;
        uint16_t ocrval = (uint16_t)(15.625 * 1000 * time);
        OCR5A = ocrval;

        disable_interrupts();
        TIMSK5 |= (1 << OCIE5A); // enable timer interrupt
        enable_interrupts();

        while(1){
          if(timer_flag){
            timer_flag = 0;
          
            amp_value amp = {0, 0};
            amp.current = adc_read() ; // TODO:Calculate RMS value
            amp.timestamp = measurement_count;

            UART_send_amp_binary(&amp);
            
            amp_array[measurement_count] = amp; // Storing amp in array
          }
        }
      }
      //QUERY MODE
      else if(sm.mode=='q'){
        amp_value amp = {0, 0};
        UART_send_amp_binary(&amp);
      }
      //CLEARING MODE
      else if(sm.mode=='c'){
        memset(amp_array, 0, sizeof(amp_array));
      }
      message_received = 0;

    }
  }

  
  
}
