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

void print_amp(amp_value amp) {
  printf("at time %ds current is %dA\n", amp.timestamp, amp.current);
}

amp_value UART_read_amp(int fd) {
  int bytes_read = 0;
  int total_bytes_read = 0;
  amp_value amp = {0, 0};

  bytes_read = read(fd, &amp, sizeof(amp_value));
  if (bytes_read == sizeof(amp_value)) {
    total_bytes_read += bytes_read;
  } else {
    perror("read");
  }

  return amp;
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

  while (1) {
    if (read_or_write) {
      amp_value amp = UART_read_amp(fd);
      print_amp(amp);
    } else {
      // cin.getline(buf, 1024);
      // int l = strlen(buf);
      // buf[l] = '\n';
      // ++l;
      // write(fd, buf, l);
    }
  }

  return 0;
}