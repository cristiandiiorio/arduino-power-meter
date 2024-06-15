#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "my_uart.h" // this includes the UART_putString and initializes it
#include "../avr_common/common.h"

#define ARRAY_SIZE 2000

void UART_send_amp_binary(amp_value *amp);

void adc_init(void);

float adc_read(void);

float calculate_rms(float *buffer, uint16_t size);



