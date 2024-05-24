#include "serial_linux.h"

using namespace std;

int serial_set_interface_attribs(int fd, int speed, int parity) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
    printf ("error %d from tcgetattr", errno);
    return -1;
  }
  switch (speed){
    case 19200: speed = B19200; break;
    case 57600: speed = B57600; break;
    case 115200: speed = B115200; break;
    case 230400: speed = B230400; break;
    case 576000: speed = B576000; break;
    case 921600: speed = B921600; break;
  default:
    printf("cannot set baudrate %d\n", speed);
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

int UART_receive_amp(int fd, amp_value* amp) {
  uint8_t* data = (uint8_t*)amp;
  size_t total_read = 0;
  while (total_read < sizeof(amp_value)) {
    ssize_t n = read(fd, data + total_read, sizeof(amp_value) - total_read);
    if (n > 0) {
      total_read += n;
    } else if (n < 0) {
      printf("Error reading from serial port\n");
      return 0;
    }
  }
  return 1;
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
  while(1) {
    char buf[1024];
    memset(buf, 0, 1024);
    if (read_or_write) {
      int nchars=read(fd, buf,1024);
      printf("%s", buf);
    } else {
      printf("WIP\n");
      // cin.getline(buf, 1024);
      // int l=strlen(buf);
      // buf[l]='\n';
      // ++l;
      // write(fd, buf, l);
    }
  }
}
