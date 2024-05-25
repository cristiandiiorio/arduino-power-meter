#include "receiver.h"

int serial_set_interface_attribs(int fd, int speed, int parity) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
    printf ("error %d from tcgetattr", errno);
    return -1;
  }
  switch (speed){
  case 19200:
    speed=B19200;
    break;
  case 57600:
    speed=B57600;
    break;
  case 115200:
    speed=B115200;
    break;
  case 230400:
    speed=B230400;
    break;
  case 576000:
    speed=B576000;
    break;
  case 921600:
    speed=B921600;
    break;
  default:
    printf("cannot sed baudrate %d\n", speed);
    return -1;
  }
  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);
  cfmakeraw(&tty);
  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);               // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;      // 8-bit chars

  if (tcsetattr (fd, TCSANOW, &tty) != 0) {
    printf ("error %d from tcsetattr", errno);
    return -1;
  }
  return 0;
}

void serial_set_blocking(int fd, int should_block) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
      printf ("error %d from tggetattr", errno);
      return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    printf ("error %d setting term attributes", errno);
}

int serial_open(const char* name) {
  int fd = open (name, O_RDWR | O_NOCTTY | O_SYNC );
  if (fd < 0) {
    printf ("error %d opening serial, fd %d\n", errno, fd);
  }
  return fd;
}

int UART_read(int fd, uint16_t *value) {
  uint8_t buffer[sizeof(uint16_t)];
  size_t total_bytes_read = 0;
  ssize_t bytes_read;

  // Continuously read until we have the required number of bytes
  while (total_bytes_read < sizeof(uint16_t)) {
    bytes_read = read(fd, buffer + total_bytes_read, sizeof(uint16_t) - total_bytes_read);
    if (bytes_read < 0) {
      perror("Error reading from UART");
      return -1;
    }
    total_bytes_read += bytes_read;
  }


  // Debug print to see the raw bytes read
  printf("Raw bytes read: %02x %02x\n", buffer[0], buffer[1]);

  // Copy the binary data into the amp_value struct
  *value =  buffer[0]  | (buffer[1] << 8); // Assuming little-endian format
  return 0;
}

// Function to read and deserialize amp_value struct from a file descriptor
int UART_read_amp_binary(int fd, amp_value *amp) {
    uint16_t timestamp;
    uint16_t current;
    // Read timestamp
    if (UART_read(fd, &timestamp) != 0) {
        return -1;
    }

    // Read current
    if (UART_read(fd, &current) != 0) {
        return -1;
    }

    amp->timestamp = timestamp;
    amp->current = current;

    return 0;
}

/*
  serial_linux <serial_file> <baudrate> <read=1, write=0>
*/

int main(int argc, const char** argv) {
  if (argc<4) {
    printf("serial_linux <serial_file> <baudrate> <read=1, write=0>\n");
  }
  const char* serial_device=argv[1];
  int baudrate=atoi(argv[2]);
  int read_or_write=atoi(argv[3]);

  int fd=serial_open(serial_device);
  serial_set_interface_attribs(fd, baudrate, 0);
  serial_set_blocking(fd, 1);

  printf("in place\n");

  amp_value amp = {0, 0};
  while (1) {
    char buf[1024];
    memset(buf, 0, 1024);
    if (read_or_write) {
      // Read and deserialize amp_value from fd
      if (UART_read_amp_binary(fd, &amp) == 0) {
        // Print the deserialized values
        printf("Timestamp: %u\n", amp.timestamp);
        printf("Current: %u\n", amp.current);
      }
    } else {
      // Write operation (commented out as per the original code)
      // cin.getline(buf, 1024);
      // int l = strlen(buf);
      // buf[l] = '\n';
      // ++l;
      // write(fd, buf, l);
    }
  }

  return 0;
}