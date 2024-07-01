#include "meter.h"

//UART Global Variables
volatile uint8_t mode;
volatile uint8_t uart_flag = 0;

//Timer Global Variables
volatile uint8_t online_flag = 0;
volatile uint8_t timer_flag = 0;
volatile uint16_t measurement_count = 0;
volatile uint8_t sensor_flag = 0;

// Running sums and counts for aggregation
volatile float minute_sum = 0;
volatile uint8_t minute_count = 0;

volatile float hour_sum = 0;
volatile uint8_t hour_count = 0;

volatile float day_sum = 0;
volatile uint8_t day_count = 0;

volatile float month_sum = 0;
volatile uint8_t month_count = 0;

//Function to update the time storage locations
void update_time_arrays(amp_value amp, amp_value* last_minute_array, amp_value* last_hour_array, amp_value* last_day_array, amp_value* last_month_array, amp_value* last_year_array) {
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


void sampling_timer_init(void){
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // set up timer with prescaler = 1024
  uint16_t ocrval = (uint16_t)(15.625); //1000 hz (come da ricevimento)
  OCR1A = ocrval;
  
  TIMSK1 |= (1 << OCIE1A); // enable timer interrupt
}

void online_mode_timer_init(uint8_t mode){
  TCCR5A = 0; 
  TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52) ; // set up timer with prescaler = 1024
  const uint8_t time = mode;
  uint16_t ocrval = (uint16_t)(15.625 * 1000 * time);
  OCR5A = ocrval;

  disable_interrupts();
  TIMSK5 |= (1 << OCIE5A); // enable timer interrupt
  enable_interrupts();
}

void detached_mode_timer_init(void){
  TCCR3A = 0;
  TCCR3B = (1 << WGM32) | (1 << CS30) | (1 << CS32); // set up timer with prescaler = 1024
  uint16_t ocrval = (uint16_t)(15.625 * 1000); //1 second
  OCR3A = ocrval;

  disable_interrupts();
  TIMSK3 |= (1 << OCIE3A); // enable timer interrupt
  enable_interrupts();

}

ISR(TIMER1_COMPA_vect) {
  sensor_flag = 1;
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
  sampling_timer_init();
  
  enable_interrupts();

  DDRB |= (1 << PB7);  //set LED_PIN as an output

  //time storage locations
  amp_value last_minute_array[SECONDS_IN_MINUTE]; // contains amp_values for the last minute
  amp_value last_hour_array[MINUTES_IN_HOUR]; // contains amp_values for the last hour
  amp_value last_day_array[HOURS_IN_DAY]; // contains amp_values for the last day
  amp_value last_month_array[DAYS_IN_MONTH]; // contains amp_values for the last month
  amp_value last_year_array[MONTHS_IN_YEAR]; // contains amp_values for the last year


  while (1) {
    //USER MODE
    if(uart_flag){
      uart_flag = 0; //reset flag

      //STOP timer3 interrupt
      TIMSK3 &= ~(1 << OCIE3A);
      TCCR3B &= ~((1 << CS32) | (1 << CS31) | (1 << CS30));

      if(mode == 'q'){
        //TODO: send all time storage locations over UART
        // Send last_minute_array
        for (int i = 0; i < SECONDS_IN_MINUTE; i++) {
          UART_send_amp_binary(&last_minute_array[i]);
          _delay_ms(5);
        }

        // Send last_hour_array
        for (int i = 0; i < MINUTES_IN_HOUR; i++) {
          UART_send_amp_binary(&last_hour_array[i]);
          _delay_ms(5);
        }

        // Send last_day_array
        for (int i = 0; i < HOURS_IN_DAY; i++) {
          UART_send_amp_binary(&last_day_array[i]);
          _delay_ms(5);
        }

        // Send last_month_array
        for (int i = 0; i < DAYS_IN_MONTH; i++) {
          UART_send_amp_binary(&last_month_array[i]);
          _delay_ms(5);
        }

        // Send last_year_array
        for (int i = 0; i < MONTHS_IN_YEAR; i++) {
          UART_send_amp_binary(&last_year_array[i]);
          _delay_ms(5);
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
        online_mode_timer_init(mode);

        float max_val = 0;
        float min_val = 0;
        float new_val = 0;
        while(1){
          if(sensor_flag){ //measuring every 1000hz
            new_val = adc_read();
            if (new_val > max_val) { //get max value
              max_val = new_val;
            }
            if (new_val < min_val || min_val == 0) { //get min value
              min_val = new_val;
            }
            sensor_flag = 0; //reset flag
          }
          if(online_flag){ //1000hz interval ended, we send the data calculated
            float current = calculate_current(min_val, max_val);
            
            amp_value amp = {0, 0};
            amp.current = current;
            amp.timestamp = measurement_count * mode;
            
            UART_send_amp_binary(&amp);

            max_val = 0; //reset max value
            min_val = 0; //reset min value
            online_flag = 0; //reset flag
          }

          sleep_cpu(); //I SLEEP
        }
      }   
    }
    //DETACHED MODE
    else{
      detached_mode_timer_init();
      
      float max_val = 0;
      float min_val = 0;
      float new_val = 0;
      while(uart_flag == 0){ //serial not connected 
        if(sensor_flag){ //measuring every 1000hz
          new_val = adc_read();
          if (new_val > max_val) { //get max value
            max_val = new_val;
          }
          if (new_val < min_val || min_val == 0) { //get min value
            min_val = new_val;
          }
          sensor_flag = 0; //reset flag
        }
        if(timer_flag){
          float current = calculate_current(min_val, max_val);
          
          amp_value amp = {0, 0};
          amp.current = current;
          amp.timestamp = measurement_count;

          update_time_arrays(amp, last_minute_array, last_hour_array, last_day_array, last_month_array, last_year_array);

          max_val = 0; //reset max value
          min_val = 0; //reset min value
          timer_flag = 0; //reset flag

          PORTB ^= (1 << PB7);  //toggle the LED_PIN (helps for debugging) 

        }

        sleep_cpu(); //I SLEEP
      }
    }
     
  }

  return 0;
}
