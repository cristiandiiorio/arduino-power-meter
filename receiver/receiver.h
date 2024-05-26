#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

typedef struct __attribute__((packed)) amp_value {
  float current; 
  uint16_t timestamp;
} amp_value;


typedef struct __attribute__((packed)) special_message {
  int payload; 
  char mode[1];
} special_message;

//! returns the descriptor of a serial port
int serial_open(const char* name);

//! sets the attributes
int serial_set_interface_attribs(int fd, int speed, int parity);

//! puts the port in blocking/nonblocking mode
void serial_set_blocking(int fd, int should_block);

void print_amp(amp_value amp);

amp_value UART_read_amp(int fd);