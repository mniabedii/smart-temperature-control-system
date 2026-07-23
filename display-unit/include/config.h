#ifndef CONFIG_H
#define CONFIG_H

#include <mega64.h>

// UI Macros
#define UI_DDR     DDRC
#define UI_PORT    PORTC
#define LED_GREEN  0
#define LED_YELLOW 1
#define LED_RED    2
#define BUZZER_PIN 3

#define BTN_DDR    DDRD
#define BTN_PORT   PORTD
#define BTN_PIN    PIND
#define BTN_MODE   0
#define BTN_UP     1
#define BTN_DOWN   2

#define UART_BAUD_51 51

#endif