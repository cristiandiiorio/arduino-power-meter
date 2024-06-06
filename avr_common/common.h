#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct __attribute__((packed)) amp_value {
  float current; 
  uint16_t timestamp;
} amp_value;


typedef struct __attribute__((packed)) special_message {
  uint16_t payload; 
  char mode;
} special_message;

#define interrupts() sei()
#define no_interrupts() cli()
