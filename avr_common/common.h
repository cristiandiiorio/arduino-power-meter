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

#define SECONDS_IN_MINUTE 60
#define MINUTES_IN_HOUR 60
#define HOURS_IN_DAY 24
#define DAYS_IN_MONTH 30
#define MONTHS_IN_YEAR 12
