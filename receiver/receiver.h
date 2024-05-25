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
  uint16_t current; 
  uint16_t timestamp;
} amp_value;

//! returns the descriptor of a serial port
int serial_open(const char* name);

//! sets the attributes
int serial_set_interface_attribs(int fd, int speed, int parity);

//! puts the port in blocking/nonblocking mode
void serial_set_blocking(int fd, int should_block);
