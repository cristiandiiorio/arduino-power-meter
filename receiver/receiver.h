#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "../avr_common/common.h"

#define blocking_status 1
#define BAUDRATE 19200

// returns the descriptor of a serial port
int serial_open(const char* name);

// sets the attributes
int serial_set_interface_attribs(int fd, int speed, int parity);

// puts the port in blocking/nonblocking mode
void serial_set_blocking(int fd, int should_block);

void print_amp(amp_value amp, int selector);

amp_value UART_read_amp(int fd);

char input_mode(void);

uint8_t get_input_sampling(void);

char input_confirmation(void);

void UART_send_special_message(int fd, char msg);

void signal_handler(int signum);

void print_query(int fd);