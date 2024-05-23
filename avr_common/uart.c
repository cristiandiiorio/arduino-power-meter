#include "uart.h"
// ********************************************************************************
// Includes
// ********************************************************************************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
// ********************************************************************************
// Macros and Defines
// ********************************************************************************
#define BAUD 19600
#define MYUBRR F_CPU/16/BAUD-1
// ********************************************************************************
// Function Prototypes
// ********************************************************************************
void usart_init(uint16_t ubrr);
char usart_getchar( void );
void usart_putchar( char data );
void usart_pstr (char *s);
unsigned char usart_kbhit(void);
int usart_putchar_printf(char var, FILE *stream);
uint8_t UART_getString(uint8_t* buf);
void UART_putString(uint8_t* buf);

static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

// ********************************************************************************
// usart Related
// ********************************************************************************
void usart_init( uint16_t ubrr) {
    // Set baud rate
    UBRR0H = (uint8_t)(ubrr>>8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */ 
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);   /* Enable RX and TX */  
}
void usart_putchar(char data) {
    // Wait for empty transmit buffer
    while ( !(UCSR0A & (_BV(UDRE0))) );
    // Start transmission
    UDR0 = data; 
}
char usart_getchar(void) {
    // Wait for incoming data
    while ( !(UCSR0A & (_BV(RXC0))) );
    // Return the data
    return UDR0;
}
unsigned char usart_kbhit(void) {
    //return nonzero if char waiting polled version
    unsigned char b;
    b=0;
    if(UCSR0A & (1<<RXC0)) b=1;
    return b;
}
void usart_pstr(char *s) {
    // loop through entire string
    while (*s) { 
        usart_putchar(*s);
        s++;
    }
}
 
// this function is called by printf as a stream handler
int usart_putchar_printf(char var, FILE *stream) {
    // translate \n to \r for br@y++ terminal
    if (var == '\n') usart_putchar('\r');
    usart_putchar(var);
    return 0;
}

void printf_init(void){
  stdout = &mystdout;
  
  // fire up the usart
  usart_init ( MYUBRR );
}

// reads a string until the first newline or 0
// returns the size read
uint8_t UART_getString(uint8_t* buf){
  uint8_t* b0=buf; //beginning of buffer
  while(1){
    uint8_t c=usart_getchar();
    *buf=c;
    ++buf;
    // reading a 0 terminates the string
    if (c==0)
      return buf-b0;
    // reading a \n  or a \r return results
    // in forcedly terminating the string
    if(c=='\n'||c=='\r'){
      *buf=0;
      ++buf;
      return buf-b0;
    }
  }
}

void UART_putString(uint8_t* buf){
  while(*buf){
    usart_putchar(*buf);
    ++buf;
  }
}



