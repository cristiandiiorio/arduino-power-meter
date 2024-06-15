#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct __attribute__((packed)) amp_value {
  float current; 
  uint16_t timestamp;
} amp_value;

#define enable_interrupts() sei()
#define disable_interrupts() cli()
