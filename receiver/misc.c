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

  // Clear input and output buffers
  if (tcflush(fd, TCIOFLUSH) != 0) {
    perror("tcflush");
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

void print_amp(amp_value amp, int selector) {
  if(selector){
    if(amp.current > 1){
      printf("At time %ds current is %.2fA\n", amp.timestamp, amp.current);
    }
    else{
      printf("At time %ds current is %.0fmA\n", amp.timestamp, amp.current*1000);
    }
  }
  else{
    printf("|%.0fmA| ", amp.current*1000);
  }
}

amp_value UART_read_amp(int fd) {
  int bytes_read = 0;
  int total_bytes_read = 0;
  amp_value amp = {0, 0};

  bytes_read = read(fd, &amp, sizeof(amp_value));
  //printf("bytes read: %d\n", bytes_read);
  if (bytes_read == sizeof(amp_value)) {
    total_bytes_read += bytes_read;
  } else {
    perror("read");
    printf("Expected to read %lu bytes, but got %d bytes\n", sizeof(amp_value), bytes_read);
  }

  return amp;
}

void UART_send_special_message(int fd, char msg) {
  ssize_t bytes_written = write(fd, &msg, sizeof(char));
  if (bytes_written < 0) {
    printf("Error writing to serial port\n");
    return;
  }
  //printf("Sent char: %x\n", msg);
}

char input_mode(void){
  char input[3];
  printf("Type o for online mode, q for query mode, c for clearing mode: ");

  if(fgets(input, sizeof(input), stdin)){
    if(input[1] == '\n' || input[1] == '\0'){ //correct length
      if ((input[0] >= 'a' && input[0] <= 'z') || (input[0] >= 'A' && input[0] <= 'Z') || (input[0] >= '0' && input[0] <= '9')) { //correct character
        if(input[0]=='o' || input[0]=='q' || input[0]=='c'){ //existing mode
          return input[0];
        }
        else {
          printf("Invalid mode entered!\n");
          return -1;
        }
      }
      else {
        printf("Invalid character entered!\n");
        return -1;
      }
    }
    else{
      printf("You have entered more than one character!\n");
      return -1;
    }
  }
  else{
    printf("Error reading input\n");
    return -1;
  }
}

uint8_t get_input_sampling(void){
  char input[4]; //Maximum of 60 seconds ==> 2 chars
  uint8_t sampling_interval;
  printf("Desired sampling interval: ");
  if (fgets(input, sizeof(input), stdin)) {
    if(input[2] == '\n' || input[2] == '\0'){ //correct length
      sampling_interval = atoi(input);
      if (sampling_interval > 60) { //correct interval
        printf("Wrong sampling interval\n");
        return 0;
      }
      return sampling_interval;
    }
    else {
      printf("You have entered more than two characters!\n");
      return 0;
    }
  } 
  else {
    printf("Error reading input\n");
    return 0;
  }
}

char input_confirmation(void){
  char input[3];
  printf("Are you sure you want to clear the array? (Y/n): ");

  if(fgets(input, sizeof(input), stdin)){
    if(input[1] == '\n' || input[1] == '\0'){ //correct length
      if ((input[0] >= 'a' && input[0] <= 'z') || (input[0] >= 'A' && input[0] <= 'Z') || (input[0] >= '0' && input[0] <= '9')) { //correct character
        if(input[0]=='Y' || input[0]=='n' ){ //existing choices
          return input[0];
        }
        else {
          printf("Invalid choice entered!\n");
          return -1;
        }
      }
      else {
        printf("Invalid character entered!\n");
        return -1;
      }
    }
    else{
      printf("You have entered more than one character!\n");
      return -1;
    }
  }
  else{
    printf("Error reading input\n");
    return -1;
  }
}

void print_query(int fd){
  // Receive and print last_minute_array
  printf("Last Minute:\n");
  amp_value amp;
  for (int i = 0; i < SECONDS_IN_MINUTE; i++) {
    amp = UART_read_amp(fd);
    print_amp(amp, 0);
    if ((i + 1) % 6 == 0) printf("\n");
  }
  printf("\n--------------------\n");
  
  // Receive and print last_hour_array
  printf("Last Hour:\n");
  for (int i = 0; i < MINUTES_IN_HOUR; i++) {
    amp = UART_read_amp(fd);
    print_amp(amp, 0);
    if ((i + 1) % 6 == 0) printf("\n");
  }
  printf("\n--------------------\n");

  // Receive and print last_day_array
  printf("Last Day:\n");
  for (int i = 0; i < HOURS_IN_DAY; i++) {
    amp = UART_read_amp(fd);
    print_amp(amp, 0);
    if ((i + 1) % 6 == 0) printf("\n");
  }
  printf("\n--------------------\n");

  // Receive and print last_month_array
  printf("Last Month:\n");
  for (int i = 0; i < DAYS_IN_MONTH; i++) {
    amp = UART_read_amp(fd);
    print_amp(amp, 0);
    if ((i + 1) % 6 == 0) printf("\n");
  }
  printf("\n--------------------\n");
  // Receive and print last_year_array
  printf("Last Year:\n");
  for (int i = 0; i < MONTHS_IN_YEAR; i++) {
    amp = UART_read_amp(fd);
    print_amp(amp, 0);
    if ((i + 1) % 6 == 0) printf("\n");
  }
  printf("\n--------------------\n");
}