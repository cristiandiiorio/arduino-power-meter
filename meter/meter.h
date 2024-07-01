#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "my_uart.h" // this includes the UART_putString and initializes it
#include "../avr_common/common.h"

#define ARRAY_SIZE 2000

#define CALIBRATION1 0.586
#define CALIBRATION2 0.0237

void UART_send_amp_binary(amp_value *amp);

void adc_init(void);

float adc_read(void);

void update_time_arrays(amp_value amp, amp_value* last_minute_array, amp_value* last_hour_array, amp_value* last_day_array, amp_value* last_month_array, amp_value* last_year_array);

float calculate_current(float min_val, float max_val);
