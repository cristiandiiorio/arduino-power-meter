#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "my_uart.h" // this includes the UART_putString and initializes it
#include "../avr_common/common.h"

#define CALIBRATION1 0.586
#define CALIBRATION2 0.0237


void UART_send_amp_binary(amp_value *amp);
void adc_init(void);
float adc_read(void);
float calculate_current(float min_val, float max_val);

void sampling_timer_init(void);
void online_mode_timer_init(uint8_t mode);
void detached_mode_timer_init(void);
void update_time_arrays(amp_value amp, amp_value* last_seconds, amp_value* last_minutes, amp_value* last_hours, amp_value* last_days, amp_value* last_months);
void query_mode_send( amp_value* last_seconds, amp_value* last_minutes, amp_value* last_hours, amp_value* last_days, amp_value* last_months);
