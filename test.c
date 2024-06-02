#include <stdint.h>
#include <stdio.h>

typedef struct __attribute__((packed)) amp_value {
  float current; 
  uint16_t timestamp;
} amp_value;

typedef struct __attribute__((packed)) special_message {
  uint16_t payload; 
  char mode;
} special_message;

int main() {
  //printf("Size of amp_value: %ld bytes\n", sizeof(amp_value));
  //printf("Size of special_message: %ld bytes\n", sizeof(special_message));
  
  char mode[2];
  printf("%ld", sizeof(mode));
  return 0;
}
