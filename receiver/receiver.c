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
  printf("at time %ds current is %fA\n", amp.timestamp, amp.current);
  // printf("%fmA\n",amp.current);
}

amp_value UART_read_amp(int fd) {
  int bytes_read = 0;
  int total_bytes_read = 0;
  amp_value amp = {0, 0};

  bytes_read = read(fd, &amp, sizeof(amp_value));
  printf("bytes read: %d\n", bytes_read);
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
  printf("Sent char: %x\n", msg);
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

int input_sampling(void){
  char input[4]; //Maximum of 60 seconds ==> 2 chars
  int sampling_interval;
  printf("Desired sampling interval: ");
  if (fgets(input, sizeof(input), stdin)) {
    if(input[2] == '\n' || input[2] == '\0'){ //correct length
      sampling_interval = atoi(input);
      if (sampling_interval < 0 || sampling_interval > 60) { //correct interval
        printf("Wrong sampling interval\n");
      }
      return sampling_interval;
    }
    else {
      printf("You have entered more than two characters!\n");
    }
  } 
  else {
    printf("Error reading input\n");
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

/*
  receiver /dev/ttyUSB0 19200
*/

int main(void) {
  const char* serial_device="/dev/ttyUSB0";
  const int baudrate = 19200;
  
  //serial setup
  int fd = serial_open(serial_device);
  serial_set_interface_attribs(fd, baudrate, 0);
  serial_set_blocking(fd, blocking_status);

  //read from stdin
  char mode = input_mode();

  // online mode 
  if (mode == 'o') {
    //read from stdin
    uint8_t sampling_interval = input_sampling();    
    
    //send sampling_interval itself, 
    //it fits into a byte since it can only go up to 60
    UART_send_special_message(fd, sampling_interval);

    //read from arduino
    while(1){
      amp_value amp = UART_read_amp(fd);
      print_amp(amp);
    }
  } 

  // query mode TODO
  else if (mode == 'q') { 
    UART_send_special_message(fd, mode);
    //TODO
    amp_value amp = UART_read_amp(fd);
    print_amp(amp);
  }

  // clearing mode
  else if (mode == 'c') {
    //read from stdin
    char confirmation = input_confirmation();
    
    if (confirmation == 'Y') {
      //send special_message to arduino
      UART_send_special_message(fd, mode);
      //read from arduino
      amp_value amp = UART_read_amp(fd);
      
      //check to see if memory has been cleared
      if(amp.current == -1){
        printf("Memory cleared\n");
      }
      else{
        printf("Memory not cleared\n");
        print_amp(amp);
      }
    }
  }

  return 0;
}