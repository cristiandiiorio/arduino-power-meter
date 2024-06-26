#include "meter.h"

//UART Global Variables
volatile uint8_t mode;
volatile uint8_t uart_flag = 0;

//Timer Global Variables
volatile uint8_t online_flag = 0;
volatile uint8_t timer_flag = 0;
volatile uint16_t measurement_count = 0;

//time 
amp_value last_minute_array[60]; //contains amp_values for the last minute
amp_value last_hour_array[60]; //contains amp_values for the last hour
amp_value last_day_array[24]; //contains amp_values for the last day
amp_value last_month_array[30]; //contains amp_values for the last month
amp_value last_year_array[12]; //contains amp_values for the last year

void UART_send_amp_binary(amp_value *amp) {
  uint8_t* amp_ptr = (uint8_t*) amp;
  int i = 0;
  while (i < sizeof(amp_value)){
    UART_putChar(amp_ptr[i]);
    i++;
  }

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

ISR(TIMER3_COMPA_vect) {
  timer_flag = 1; //set the flag to indicate timer overflow
  measurement_count++;
}


ISR(TIMER5_COMPA_vect) {
  online_flag = 1; //set the flag to indicate timer overflow
  measurement_count++;
}


ISR(USART0_RX_vect) {
  uart_flag = 1;
  mode = UDR0; //read byte from UART representing MODE
}


int main(void) {
  UART_init();
  adc_init();
  
  enable_interrupts();

  while (1) {
    //DETACHED MODE
    TCCR3A = 0;
    TCCR3B = (1 << WGM32) | (1 << CS30) | (1 << CS32); // set up timer with prescaler = 1024
    uint16_t ocrval = (uint16_t)(15.625 * 1000); //1 second
    OCR3A = ocrval;

    disable_interrupts();
    TIMSK3 |= (1 << OCIE3A); // enable timer interrupt
    enable_interrupts();

    while(!uart_flag){ //serial not connected
      if(timer_flag){
        timer_flag = 0; //reset flag

        amp_value amp = {0, 0};
        amp.current = adc_read(); // TODO:Calculate RMS value
        amp.timestamp = measurement_count;
        UART_send_amp_binary(&amp);

        
      }

      sleep_cpu(); //I SLEEP
    }

    //--------------------------------------------------------------//
    //USER MODE
    uart_flag = 0; //reset flag

    //disable && stop timer3 interrupt
    TIMSK3 &= ~(1 << OCIE3A);
    TCCR3B &= ~((1 << CS32) | (1 << CS31) | (1 << CS30));

    if(mode == 'q'){
      //TODO: send all time storage locations over UART
      amp_value amp = {2, 2};
      UART_send_amp_binary(&amp);
    }
    else if(mode == 'c'){
      //TODO: clear all time storage locations and send confirmation over UART
      //memset(amp_array, 0, sizeof(amp_array)); 
      amp_value amp = {-1, 0}; // -1 indicates memory cleared
      UART_send_amp_binary(&amp);
    }
    else{ //mode == 'o'
      TCCR5A = 0; 
      TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52) ; // set up timer with prescaler = 1024
      const uint8_t time = mode;
      uint16_t ocrval = (uint16_t)(15.625 * 1000 * time);
      OCR5A = ocrval;

      disable_interrupts();
      TIMSK5 |= (1 << OCIE5A); // enable timer interrupt
      enable_interrupts();

      while(1){
        if(online_flag){
          amp_value amp = {0, 0};
          amp.current = adc_read(); // TODO:Calculate RMS value
          amp.timestamp = measurement_count  * time;
          UART_send_amp_binary(&amp);

          online_flag = 0; //reset flag
        }

        sleep_cpu(); //I SLEEP
      }
    }    
  }

  return 0;
}
