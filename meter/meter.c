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

ISR(TIMER1_COMPA_vect) { //1000hz timer for sampling
  sensor_flag = 1;
}

ISR(TIMER3_COMPA_vect) { //1sec timer for detached mode
  timer_flag = 1; //set the flag to indicate timer overflow
  measurement_count++;
}

ISR(TIMER5_COMPA_vect) { //variable timer for online mode
  online_flag = 1; //set the flag to indicate timer overflow
  measurement_count++;
}

ISR(USART0_RX_vect) { //interrupt when pc sends data
  uart_flag = 1;
  mode = UDR0; //read byte from UART representing MODE
}


int main(void) {
  UART_init();
  adc_init();
  sampling_timer_init();
  
  enable_interrupts();

  DDRB |= (1 << PB7);  //set LED_PIN as an output

  //time arrays
  amp_value last_seconds[SECONDS_IN_MINUTE]; // contains amp_values for the last 60 seconds
  amp_value last_minutes[MINUTES_IN_HOUR]; // contains amp_values for the last 60 minutes
  amp_value last_hours[HOURS_IN_DAY]; // contains amp_values for the last 24 hours
  amp_value last_days[DAYS_IN_MONTH]; // contains amp_values for the last 30 days
  amp_value last_months[MONTHS_IN_YEAR]; // contains amp_values for the last 12 months
  //clear all time arrays
  memset(last_seconds, 0, sizeof(last_seconds));
  memset(last_minutes, 0, sizeof(last_minutes));
  memset(last_hours, 0, sizeof(last_hours));
  memset(last_days, 0, sizeof(last_days));
  memset(last_months, 0, sizeof(last_months));

  while (1) {
    //USER MODE
    if(uart_flag){
      uart_flag = 0; //reset flag

      //stop timer3 interrupt
      TIMSK3 &= ~(1 << OCIE3A);
      TCCR3B &= ~((1 << CS32) | (1 << CS31) | (1 << CS30));

      if(mode == 'q'){
        query_mode_send(last_seconds, last_minutes, last_hours, last_days, last_months);
      }
      else if(mode == 'c'){
        //clears all time arrays and send confirmation over UART
        memset(last_seconds, 0, sizeof(last_seconds));
        memset(last_minutes, 0, sizeof(last_minutes));
        memset(last_hours, 0, sizeof(last_hours));
        memset(last_days, 0, sizeof(last_days));
        memset(last_months, 0, sizeof(last_months));
        
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
          if(online_flag){ //1000hz interval ended, calculate the current and send it
            float current = calculate_current(min_val, max_val);
            
            amp_value amp = {0, 0};
            amp.current = current;
            amp.timestamp = measurement_count * mode; //*mode is useful when the online interval is >= 1 second
            
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
        if(timer_flag){ //1000hz interval ended, calculate the current and save it 
          float current = calculate_current(min_val, max_val);
          
          amp_value amp = {0, 0};
          amp.current = current;
          amp.timestamp = measurement_count;

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


//Function to update the time arrays
void update_time_arrays(amp_value amp, amp_value* last_seconds, amp_value* last_minutes, amp_value* last_hours, amp_value* last_days, amp_value* last_months) {
  // Update last_seconds
  last_seconds[seconds_index] = amp;
  seconds_index++;

  if (seconds_index == SECONDS_IN_MINUTE) { //1 minute has passed
    // Calculate average for the last minute
    float minute_sum = 0;
    for(uint8_t i = 0; i < SECONDS_IN_MINUTE; i++){
      minute_sum += last_seconds[i].current;
    }
    float minute_avg = minute_sum / SECONDS_IN_MINUTE;

    // Update last_minutes
    amp_value minutes_amp = {minute_avg, measurement_count};
    last_minutes[minutes_index] = minutes_amp;
    minutes_index++;

    // Reset seconds_index
    seconds_index = 0;
  }

  if (minutes_index == MINUTES_IN_HOUR) { //1 hour has passed
    // Calculate average for the last hour
    float hour_sum = 0;
    for(uint8_t i = 0; i < MINUTES_IN_HOUR; i++){
      hour_sum += last_minutes[i].current;
    }
    float hour_avg = hour_sum / MINUTES_IN_HOUR;

    // Update last_hours
    amp_value hours_amp = {hour_avg, measurement_count};
    last_hours[hours_index] = hours_amp;
    hours_index++;

    // Reset minutes_index
    minutes_index = 0;
  }

  if (hours_index == HOURS_IN_DAY) { //1 day has passed
    // Calculate average for the last day
    float day_sum = 0;
    for(uint8_t i = 0; i < HOURS_IN_DAY; i++){
      day_sum += last_hours[i].current;
    }
    float day_avg = day_sum / HOURS_IN_DAY;

    // Update last_days
    amp_value month_amp = {day_avg, measurement_count};
    last_days[days_index] = month_amp;
    days_index++;

    // Reset hours_index
    hours_index = 0;
  }

  if (days_index == DAYS_IN_MONTH) { //1 month has passed
    // Calculate average for the last month
    float month_sum = 0;
    for(uint8_t i = 0; i < DAYS_IN_MONTH; i++){
      month_sum += last_days[i].current;
    }
    float month_avg = month_sum / DAYS_IN_MONTH;

    // Update last_months
    amp_value year_amp = {month_avg, measurement_count};
    last_months[months_index] = year_amp;

    // Reset month_sum and days_index
    days_index = 0;
  }
}