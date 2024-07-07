#include "receiver.h"

#define blocking_status 1
#define BAUDRATE 19200

int fd;

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

  while(1){
    //read from stdin
    char mode = input_mode();

    // online mode 
    if (mode == 'o') {
      //read from stdin
      uint8_t sampling_interval = get_input_sampling();    
      if(sampling_interval == 0){      
        return -1;
      }

      //send sampling_interval itself, 
      //it fits into a byte since it can only go up to 60
      UART_send_special_message(fd, sampling_interval);

      //read data from arduino
      while(1){
        amp_value amp = UART_read_amp(fd);
        print_amp(amp,1);
      }
    } 

    // query mode TODO
    else if (mode == 'q') { 
      // receive all time storage locations
      UART_send_special_message(fd, mode);
      // print the data received  
      print_query(fd);
    }

    // clearing mode
    else if (mode == 'c') {
      //read from stdin
      char confirmation = input_confirmation();
      
      if (confirmation == 'Y') {
        //send special_message to arduino to clear data
        UART_send_special_message(fd, mode);
        //read response from arduino to check for confirmation
        amp_value amp = UART_read_amp(fd);
        
        //check to see if memory has been cleared
        if(amp.current == -1){
          printf("Memory cleared\n");
        }
        else{
          printf("Memory not cleared\n");
        }
      }
    }
  }

  close(fd);

  return 0;
}


void signal_handler(int signum){
  if(signum == SIGINT){
    fprintf(stderr,"Exiting program after CTRL+C \n");
    close(fd);
    exit(EXIT_SUCCESS);
  }
}

