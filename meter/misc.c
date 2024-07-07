#include "meter.h"

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

//Function to calculate the calibrated rms current
float calculate_current(float min_val, float max_val){
  float sample = ((max_val - min_val)*5)/1024; //5,1024 are the Vref and the ADC resolution
  sample = sample * 0.707; //calculate RMS value (0.707 = sqrt(2)/2)
  float calibrated_sample = sample * CALIBRATION1 + CALIBRATION2; //calibration
  if (calibrated_sample < 0.03) {
    calibrated_sample = 0;
  }
  return calibrated_sample;
}



void query_mode_send(amp_value* last_seconds, amp_value* last_minutes, amp_value* last_hours, amp_value* last_days, amp_value* last_months){
  // Send last_seconds
  for (int i = 0; i < SECONDS_IN_MINUTE; i++) {
    UART_send_amp_binary(&last_seconds[i]);
    _delay_ms(5);
  }

  // Send last_minutes
  for (int i = 0; i < MINUTES_IN_HOUR; i++) {
    UART_send_amp_binary(&last_minutes[i]);
    _delay_ms(5);
  }

  // Send last_hours
  for (int i = 0; i < HOURS_IN_DAY; i++) {
    UART_send_amp_binary(&last_hours[i]);
    _delay_ms(5);
  }

  // Send last_days
  for (int i = 0; i < DAYS_IN_MONTH; i++) {
    UART_send_amp_binary(&last_days[i]);
    _delay_ms(5);
  }

  // Send last_months
  for (int i = 0; i < MONTHS_IN_YEAR; i++) {
    UART_send_amp_binary(&last_months[i]);
    _delay_ms(5);
  }
}