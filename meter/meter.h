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

uint16_t adc_read(void);

void update_time_arrays(amp_value amp, amp_value* last_seconds, amp_value* last_minutes, amp_value* last_hours, amp_value* last_days, amp_value* last_months);

uint16_t calculate_current(uint16_t min_val, uint16_t max_val);
