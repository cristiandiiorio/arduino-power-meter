#include "receiver.h"

#define blocking_status 1

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
  printf("at time %ds current is %.2fA\n", amp.timestamp, amp.current);
  // printf("%fmA\n",amp.current);
}

amp_value UART_read_amp(int fd) {
  int bytes_read = 0;
  int total_bytes_read = 0;
  amp_value amp = {0, 0};

  bytes_read = read(fd, &amp, sizeof(amp_value));
  // printf("bytes read: %d\n", bytes_read);
  if (bytes_read == sizeof(amp_value)) {
    total_bytes_read += bytes_read;
  } else {
    perror("read");
    printf("Expected to read %lu bytes, but got %d bytes\n", sizeof(amp_value), bytes_read);

  }

  return amp;
}

void UART_send_special_message(int fd, special_message *msg) {
  uint8_t* msg_ptr = (uint8_t*) msg;
  int i = 0;
  while(i < sizeof(special_message)){
    write(fd, &msg_ptr[i], sizeof(uint8_t));
    i++;
  }
}

void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

/*
  serial_linux <serial_file> <baudrate> <read=1, write=0>
*/

int main(int argc, const char** argv) {
  if (argc < 3) {
    printf("serial_linux <serial_file> <baudrate>\n");
  }
  const char* serial_device=argv[1];
  int baudrate=atoi(argv[2]);
  
  char mode;
  int input_check = 0;
  while(input_check == 0){
    printf("o for online mode, q for query mode, c for clearing mode: ");
    mode = getchar();
    if(mode=='o' || mode=='q' || mode=='c'){
      input_check = 1;
    }
    else {
      printf("That mode does not exist\n");
    }
    clear_input_buffer();
  }  
    
  //serial setup
  int fd = serial_open(serial_device);
  serial_set_interface_attribs(fd, baudrate, 0);
  serial_set_blocking(fd, blocking_status);
  
  // online mode 
  if (mode == 'o') {
    // user input
    char input[10];
    int sampling_interval;
    printf("desired sampling interval: ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
      sampling_interval = atoi(input);
      if (sampling_interval < 0) {
        printf("Wrong sampling interval\n");
      }
    } 
    else {
      printf("Error reading input\n");
    }
    //send special_message to arduino
    special_message sm = {sampling_interval, mode};
    UART_send_special_message(fd, &sm);

    while(1){
      //read from arduino
      printf("Reading from arduino\n");
      amp_value amp = UART_read_amp(fd);
      print_amp(amp);
    }
  } 

  // query mode
  else if (mode == 'q') { 

  }

  // clearing mode
  else if (mode == 'c') {
    char confirmation;
    printf("Are you sure you want to clear the array? (y/n): ");
    confirmation = getchar();
    
    if (confirmation == 'y') {
      special_message sm = {0, mode};
      UART_send_special_message(fd, &sm);
    }
    else {
      printf("Wrong confirmation\n");
    }
    clear_input_buffer();
  }

  return 0;
}