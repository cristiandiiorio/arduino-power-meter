#include <stdint.h>
#include <stdio.h>

typedef struct __attribute__((packed)) amp_value {
    uint16_t timestamp;
    uint16_t current; 
} amp_value;

int main() {
    printf("Size of amp_value: %ld bytes\n", sizeof(amp_value));
    printf("Size of pointer to amp_value: %ld bytes\n", sizeof(amp_value*));
    return 0;
}
