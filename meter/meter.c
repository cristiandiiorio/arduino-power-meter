#include "meter.h"

//UART Global Variables
volatile uint8_t mode;
volatile uint8_t uart_flag = 0;

//Timer Global Variables
volatile uint8_t online_flag = 0;
volatile uint8_t timer_flag = 0;
volatile uint16_t measurement_count = 0;

//time storage locations
amp_value last_minute_array[SECONDS_IN_MINUTE]; // contains amp_values for the last minute
amp_value last_hour_array[MINUTES_IN_HOUR]; // contains amp_values for the last hour
amp_value last_day_array[HOURS_IN_DAY]; // contains amp_values for the last day
amp_value last_month_array[DAYS_IN_MONTH]; // contains amp_values for the last month
amp_value last_year_array[MONTHS_IN_YEAR]; // contains amp_values for the last year

// Running sums and counts for aggregation
float minute_sum = 0;
uint8_t minute_count = 0;

float hour_sum = 0;
uint8_t hour_count = 0;

float day_sum = 0;
uint8_t day_count = 0;

float month_sum = 0;
uint8_t month_count = 0;


//Function to send amp_values over UART
void UART_send_amp_binary(amp_value *amp) {
  uint8_t* amp_ptr = (uint8_t*) amp;
  int i = 0;
  while (i < sizeof(amp_value)){
    UART_putChar(amp_ptr[i]);
    i++;
  }

}

//Function to initialize the ADC
void adc_init(void) {
  // Select Vref=AVcc
  ADMUX |= (1 << REFS0);
  ADMUX &= ~(1 << REFS1);

  // Set ADC prescaler to 128 for 16 MHz clock (125 kHz ADC clock)
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // Enable ADC
  ADCSRA |= (1 << ADEN);
}

//Function to read ADC channel 0
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

//Function to update the time storage locations
void update_time_arrays(amp_value amp) {
  // Update last_minute_array
  last_minute_array[measurement_count % SECONDS_IN_MINUTE] = amp;
  minute_sum += amp.current;
  minute_count++;

  if (minute_count == SECONDS_IN_MINUTE) {
    // Calculate average for the last minute
    float minute_avg = minute_sum / SECONDS_IN_MINUTE;

    // Update last_hour_array
    amp_value hour_amp = {minute_avg, measurement_count / SECONDS_IN_MINUTE};
    last_hour_array[(measurement_count / SECONDS_IN_MINUTE) % MINUTES_IN_HOUR] = hour_amp;
    hour_sum += minute_avg;
    hour_count++;

    // Reset minute_sum and minute_count
    minute_sum = 0;
    minute_count = 0;
  }

  if (hour_count == MINUTES_IN_HOUR) {
    // Calculate average for the last hour
    float hour_avg = hour_sum / MINUTES_IN_HOUR;

    // Update last_day_array
    amp_value day_amp = {hour_avg, measurement_count / ((uint64_t)SECONDS_IN_MINUTE * MINUTES_IN_HOUR)};
    last_day_array[(measurement_count / ((uint64_t)SECONDS_IN_MINUTE * MINUTES_IN_HOUR)) % HOURS_IN_DAY] = day_amp;
    day_sum += hour_avg;
    day_count++;

    // Reset hour_sum and hour_count
    hour_sum = 0;
    hour_count = 0;
  }

  if (day_count == HOURS_IN_DAY) {
    // Calculate average for the last day
    float day_avg = day_sum / HOURS_IN_DAY;

    // Update last_month_array
    amp_value month_amp = {day_avg, measurement_count / ((uint64_t)SECONDS_IN_MINUTE * MINUTES_IN_HOUR * HOURS_IN_DAY)};
    last_month_array[(measurement_count / ((uint64_t)SECONDS_IN_MINUTE * MINUTES_IN_HOUR * HOURS_IN_DAY)) % DAYS_IN_MONTH] = month_amp;
    month_sum += day_avg;
    month_count++;

    // Reset day_sum and day_count
    day_sum = 0;
    day_count = 0;
  }

  if (month_count == DAYS_IN_MONTH) {
    // Calculate average for the last month
    float month_avg = month_sum / DAYS_IN_MONTH;

    // Update last_year_array
    amp_value year_amp = {month_avg, measurement_count / ((uint64_t)SECONDS_IN_MINUTE * MINUTES_IN_HOUR * HOURS_IN_DAY * DAYS_IN_MONTH)};
    last_year_array[(measurement_count / ((uint64_t)SECONDS_IN_MINUTE * MINUTES_IN_HOUR * HOURS_IN_DAY * DAYS_IN_MONTH)) % MONTHS_IN_YEAR] = year_amp;

    // Reset month_sum and month_count
    month_sum = 0;
    month_count = 0;
  }
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

  DDRB |= (1 << PB7);  //set LED_PIN as an output

  while (1) {
    //USER MODE
    if(uart_flag){
      uart_flag = 0; //reset flag

      //disable && stop timer3 interrupt
      TIMSK3 &= ~(1 << OCIE3A);
      TCCR3B &= ~((1 << CS32) | (1 << CS31) | (1 << CS30));

      if(mode == 'q'){
        //TODO: send all time storage locations over UART
        // Send last_minute_array
        for (int i = 0; i < SECONDS_IN_MINUTE; i++) {
          UART_send_amp_binary(&last_minute_array[i]);
        }

        // Send last_hour_array
        for (int i = 0; i < MINUTES_IN_HOUR; i++) {
          UART_send_amp_binary(&last_hour_array[i]);
        }

        // Send last_day_array
        for (int i = 0; i < HOURS_IN_DAY; i++) {
          UART_send_amp_binary(&last_day_array[i]);
        }

        // Send last_month_array
        for (int i = 0; i < DAYS_IN_MONTH; i++) {
          UART_send_amp_binary(&last_month_array[i]);
        }

        // Send last_year_array
        for (int i = 0; i < MONTHS_IN_YEAR; i++) {
          UART_send_amp_binary(&last_year_array[i]);
        }
      }
      else if(mode == 'c'){
        //clears all time storage locations and send confirmation over UART
        memset(last_minute_array, 0, sizeof(last_minute_array));
        memset(last_hour_array, 0, sizeof(last_hour_array));
        memset(last_day_array, 0, sizeof(last_day_array));
        memset(last_month_array, 0, sizeof(last_month_array));
        memset(last_year_array, 0, sizeof(last_year_array));
        
        amp_value amp = {-1, 0}; // -1 indicates memory cleared
        UART_send_amp_binary(&amp); //send confirmation message
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
    //DETACHED MODE
    else{
      TCCR3A = 0;
      TCCR3B = (1 << WGM32) | (1 << CS30) | (1 << CS32); // set up timer with prescaler = 1024
      uint16_t ocrval = (uint16_t)(15.625 * 1000); //1 second
      OCR3A = ocrval;

      disable_interrupts();
      TIMSK3 |= (1 << OCIE3A); // enable timer interrupt
      enable_interrupts();

      while(uart_flag == 0){ //serial not connected 
        if(timer_flag){
          timer_flag = 0; //reset flag

          amp_value amp = {0, 0};
          amp.current = adc_read(); // TODO:Calculate RMS value
          amp.timestamp = measurement_count;

          //TODO: Storing amp in the right array
          update_time_arrays(amp);
          PORTB ^= (1 << PB7);  //toggle the LED_PIN

        }

        sleep_cpu(); //I SLEEP
      }
    }
     
  }

  return 0;
}
