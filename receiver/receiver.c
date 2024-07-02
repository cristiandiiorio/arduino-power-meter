#include "receiver.h"

#define blocking_status 1
#define BAUDRATE 19200

int fd;

void signal_handler(int signum){
  if(signum == SIGINT){
    fprintf(stderr,"Exiting program after CTRL+C \n");
    close(fd);
    exit(EXIT_SUCCESS);
  }
}


/*
  receiver /dev/ttyUSB0 19200
*/

int main(int argc, const char** argv) {
  signal(SIGINT, signal_handler);
  if (argc < 2) {
    printf("receiver <serial_file>\n");
    return -1;
  }
  const char* serial_device = argv[1];
  
  //serial setup
  fd = serial_open(serial_device);
  serial_set_interface_attribs(fd, BAUDRATE, 0);
  serial_set_blocking(fd, blocking_status);

  //read from stdin
  char mode = input_mode();

  // online mode 
  if (mode == 'o') {
    //read from stdin
    uint8_t sampling_interval = input_sampling();    
    if(sampling_interval == 0){      
      return -1;
    }

    //send sampling_interval itself, 
    //it fits into a byte since it can only go up to 60
    UART_send_special_message(fd, sampling_interval);

    //read from arduino
    while(1){
      amp_value amp = UART_read_amp(fd);
      print_amp(amp,1);
    }
  } 

  // query mode TODO
  else if (mode == 'q') { 
    UART_send_special_message(fd, mode);
    //TODO: receive all time storage locations and let the user choose which one to query
    
    // Receive all time storage locations
    amp_value amp;
    
    printf("Last Hour:\n");
    for (int i = 0; i < MINUTES_IN_HOUR; i++) {
      amp = UART_read_amp(fd);
      print_amp(amp, 0);
      if ((i + 1) % 6 == 0) printf("\n");
    }
    printf("\n--------------------\n");
    printf("Last Day:\n");
    for (int i = 0; i < HOURS_IN_DAY; i++) {
      amp = UART_read_amp(fd);
      print_amp(amp, 0);
      if ((i + 1) % 6 == 0) printf("\n");
    }
    printf("\n--------------------\n");
    printf("Last Month:\n");
    for (int i = 0; i < DAYS_IN_MONTH; i++) {
      amp = UART_read_amp(fd);
      print_amp(amp, 0);
      if ((i + 1) % 6 == 0) printf("\n");
    }
    printf("\n--------------------\n");
    printf("Last Year:\n");
    for (int i = 0; i < MONTHS_IN_YEAR; i++) {
      amp = UART_read_amp(fd);
      print_amp(amp, 0);
      if ((i + 1) % 6 == 0) printf("\n");
    }
    printf("\n--------------------\n");
    
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
      if(amp.current == 13){
        printf("Memory cleared\n");
      }
      else{
        printf("Memory not cleared\n");
      }
    }
  }

  close(fd);

  return 0;
}