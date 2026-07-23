#include "../include/config.h"
#include "../include/uart.h"

void uart_init(void)
{
    UBRR0H = 0;
    UBRR0L = UART_BAUD_51;
    UCSR0B = (1<<RXCIE0) | (1<<TXEN0) | (1<<RXEN0);
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

void uart_putchar(char c)
{
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = c;
}

void uart_putstring(char* str)
{
    while (*str) uart_putchar(*str++);
}