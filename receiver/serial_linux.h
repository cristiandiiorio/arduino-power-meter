#pragma once
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
#include <iostream>
#include <string> 
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
  
  //! returns the descriptor of a serial port
  int serial_open(const char* name);

  //! sets the attributes
  int serial_set_interface_attribs(int fd, int speed, int parity);
  
  //! puts the port in blocking/nonblocking mode
  void serial_set_blocking(int fd, int should_block);

#ifdef __cplusplus
}
#endif

typedef struct {
    uint16_t timestamp;
    uint16_t current;
} amp_value;

