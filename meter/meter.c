#include "meter.h"

//UART Global Variables
volatile uint8_t mode;
volatile uint8_t uart_flag = 0;
//Timer Global Variables
volatile uint8_t online_flag = 0;
volatile uint8_t timer_flag = 0;
volatile uint16_t measurement_count = 0;
volatile uint8_t sensor_flag = 0;

//time arrays counters
volatile uint8_t seconds_index = 0;
volatile uint8_t minutes_index = 0;
volatile uint8_t hours_index = 0;
volatile uint8_t days_index = 0;
volatile uint8_t months_index = 0;
//time arrays sums
volatile uint16_t minute_sum = 0;
volatile uint16_t hour_sum = 0;
volatile uint16_t day_sum = 0;
volatile uint16_t month_sum = 0;
volatile uint16_t year_sum = 0;

//Function to update the time storage locations
void update_time_arrays(amp_value amp, amp_value* last_seconds, amp_value* last_minutes, amp_value* last_hours, amp_value* last_days, amp_value* last_months) {
  // Update last_seconds
  last_seconds[seconds_index] = amp;
  minute_sum += amp.current;
  seconds_index++;

  if (seconds_index >= SECONDS_IN_MINUTE) { //1 minute has passed
    // Calculate average for the last minute
    uint16_t minute_avg = minute_sum / SECONDS_IN_MINUTE;
    // Update last_minutes
    amp_value hour_amp = {minute_avg, measurement_count};
    last_minutes[minutes_index] = hour_amp;
    hour_sum += minute_avg;
    minutes_index++;

    // Reset minute_sum and seconds_index
    minute_sum = 0;
    seconds_index = 0;

    if (minutes_index >= MINUTES_IN_HOUR) {
      // Calculate average for the last hour
      uint16_t hour_avg = hour_sum / MINUTES_IN_HOUR;

      // Update last_hours
      amp_value day_amp = {hour_avg, measurement_count};
      last_hours[hours_index] = day_amp;
      day_sum += hour_avg;
      hours_index++;

      // Reset hour_sum and minutes_index
      hour_sum = 0;
      minutes_index = 0;

      if (hours_index >= HOURS_IN_DAY) {
        // Calculate average for the last day
        uint16_t day_avg = day_sum / HOURS_IN_DAY;

        // Update last_days
        amp_value month_amp = {day_avg, measurement_count};
        last_days[days_index] = month_amp;
        month_sum += day_avg;
        days_index++;

        // Reset day_sum and hours_index
        day_sum = 0;
        hours_index = 0;

        if (days_index >= DAYS_IN_MONTH) {
          // Calculate average for the last month
          uint16_t month_avg = month_sum / DAYS_IN_MONTH;

          // Update last_months
          amp_value year_amp = {month_avg, measurement_count};
          last_months[months_index] = year_amp;
          year_sum += month_avg;
          months_index++;

          // Reset month_sum and days_index
          month_sum = 0;
          days_index = 0;
        }
      }
    }
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
  amp_value last_seconds[SECONDS_IN_MINUTE]; // contains amp_values for the last 60 seconds (not requested)
  amp_value last_minutes[MINUTES_IN_HOUR]; // contains amp_values for the last 60 minutes
  amp_value last_hours[HOURS_IN_DAY]; // contains amp_values for the last 24 hours
  amp_value last_days[DAYS_IN_MONTH]; // contains amp_values for the last 30 days
  amp_value last_months[MONTHS_IN_YEAR]; // contains amp_values for the last 12 months

  while (1) {
    //USER MODE
    if(uart_flag){
      uart_flag = 0; //reset flag

      //STOP timer3 interrupt
      TIMSK3 &= ~(1 << OCIE3A);
      TCCR3B &= ~((1 << CS32) | (1 << CS31) | (1 << CS30));

      if(mode == 'q'){
        // Send last_minutes
        for (uint8_t i = 0; i < MINUTES_IN_HOUR; i++) {
          UART_send_amp_binary(&last_minutes[i]);
          _delay_ms(5);
        }

        // Send last_hours
        for (uint8_t i = 0; i < HOURS_IN_DAY; i++) {
          UART_send_amp_binary(&last_hours[i]);
          _delay_ms(5);
        }

        // Send last_days
        for (uint8_t i = 0; i < DAYS_IN_MONTH; i++) {
          UART_send_amp_binary(&last_days[i]);
          _delay_ms(5);
        }

        // Send last_months
        for (uint8_t i = 0; i < MONTHS_IN_YEAR; i++) {
          UART_send_amp_binary(&last_months[i]);
          _delay_ms(5);
        }

      }
      else if(mode == 'c'){
        disable_interrupts();
        //clears all time storage locations and send confirmation over UART
        memset(last_seconds, 0, sizeof(last_seconds));
        memset(last_minutes, 0, sizeof(last_minutes));
        memset(last_hours, 0, sizeof(last_hours));
        memset(last_days, 0, sizeof(last_days));
        memset(last_months, 0, sizeof(last_months));
        enable_interrupts();

        amp_value amp = {13, 0}; // 13 indicates memory cleared
        UART_send_amp_binary(&amp); //send confirmation message
      }
      else{ //mode == 'o'
        online_mode_timer_init(mode);

        uint16_t max_val = 0;
        uint16_t min_val = 0;
        uint16_t new_val = 0;
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
            uint16_t current = calculate_current(min_val, max_val);
            
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
      
      uint16_t max_val = 0;
      uint16_t min_val = 0;
      uint16_t new_val = 0;
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
          uint16_t current = calculate_current(min_val, max_val);
          
          amp_value amp = {0, 0};
          amp.current = current;
          amp.timestamp = measurement_count;

          //save amp_value
          update_time_arrays(amp, last_seconds, last_minutes, last_hours, last_days, last_months);

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
