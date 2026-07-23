#include <alcd.h>
#include <stdio.h>
#include <string.h>
#include "../include/config.h"
#include "../include/uart.h"

// System timers
volatile unsigned int uart_timeout = 0;
volatile unsigned int buzzer_timer = 0;
volatile unsigned int button_timer = 0;
unsigned char buzzer_state = 0;

// UART reception
volatile char rx_buffer[40];
volatile unsigned char rx_index = 0;
volatile unsigned char message_ready = 0;

// Parsed data
int current_temp = 0;
int current_fan = 0;
char current_state_str[15];

// Manual mode
unsigned char is_auto = 1;
int manual_speed = 0; 
unsigned char last_mode_state = 1;
unsigned char last_up_state = 1;
unsigned char last_down_state = 1;

interrupt [TIM0_COMP] void timer0_comp_isr(void)
{
    if (uart_timeout < 4000) uart_timeout++;
    buzzer_timer++;
    button_timer++;
}

interrupt [USART0_RXC] void usart0_rx_isr(void)
{
    char data = UDR0;
    if (data == '\n')
    {
        rx_buffer[rx_index] = '\0'; 
        message_ready = 1;
        rx_index = 0; 
        uart_timeout = 0; 
    }
    else if (data != '\r')
    {
        if (rx_index < 39) rx_buffer[rx_index++] = data;
    }
}

void send_motor_command(void)
{
    char tx_buf[15];
    if (is_auto) 
    {
        uart_putstring("A\n");
    } 
    else 
    {
        sprintf(tx_buf, "M:%d\n", manual_speed);
        uart_putstring(tx_buf);
    }
}

void main(void)
{
    char lcd_buf[17];
    char display_state[6];
    char error_shown = 0;

    UI_DDR |= (1<<LED_GREEN) | (1<<LED_YELLOW) | (1<<LED_RED) | (1<<BUZZER_PIN);
    UI_PORT = 0x00; 

    BTN_DDR &= ~((1<<BTN_MODE) | (1<<BTN_UP) | (1<<BTN_DOWN));
    BTN_PORT |= (1<<BTN_MODE) | (1<<BTN_UP) | (1<<BTN_DOWN); 

    lcd_init(16);
    lcd_clear();
    lcd_putsf("SYSTEM STARTING");

    uart_init();

    // Timer 0 configuration
    TCCR0 = 0x0B; 
    OCR0 = 124;
    TIMSK = 0x02; 
    
    #asm("sei")

    while (1)
    {                
        if (button_timer >= 50)
        {
            unsigned char curr_mode, curr_up, curr_down;
            button_timer = 0;
            
            curr_mode = (BTN_PIN & (1<<BTN_MODE)) ? 1 : 0;
            curr_up = (BTN_PIN & (1<<BTN_UP)) ? 1 : 0;
            curr_down = (BTN_PIN & (1<<BTN_DOWN)) ? 1 : 0;
            
            if (curr_mode == 0 && last_mode_state == 1)
            {
                is_auto = !is_auto;
                if (!is_auto) 
                {
                    if (current_fan >= 75) manual_speed = 75;
                    else if (current_fan >= 50) manual_speed = 50;
                    else if (current_fan >= 25) manual_speed = 25;
                    else manual_speed = 0;
                }
                send_motor_command();
            }
            
            if (!is_auto)
            {
                if (curr_up == 0 && last_up_state == 1)
                {
                    if (manual_speed == 0) manual_speed = 25;
                    else if (manual_speed == 25) manual_speed = 50;
                    else if (manual_speed == 50) manual_speed = 75;
                    else if (manual_speed == 75) manual_speed = 100;
                    send_motor_command();
                }
                if (curr_down == 0 && last_down_state == 1)
                {
                    if (manual_speed == 100) manual_speed = 75;
                    else if (manual_speed == 75) manual_speed = 50;
                    else if (manual_speed == 50) manual_speed = 25;
                    else if (manual_speed == 25) manual_speed = 0;
                    send_motor_command();
                }
            }
            
            last_mode_state = curr_mode;
            last_up_state = curr_up;
            last_down_state = curr_down;
        }

        if (uart_timeout >= 3000)
        {
            if (error_shown == 0)
            {
                lcd_clear();
                lcd_putsf("UART ERROR");
                UI_PORT &= ~((1<<LED_GREEN) | (1<<LED_YELLOW) | (1<<LED_RED) | (1<<BUZZER_PIN));
                error_shown = 1;
            }
        }
        else
        {
            error_shown = 0;

            if (message_ready == 1)
            {
                sscanf((char*)rx_buffer, "T:%d,F:%d,S:%s\n", &current_temp, &current_fan, current_state_str);
                message_ready = 0;

                UI_PORT &= ~((1<<LED_GREEN) | (1<<LED_YELLOW) | (1<<LED_RED)); 
                
                if (strcmp(current_state_str, "NORMAL") == 0) 
                {
                    UI_PORT |= (1<<LED_GREEN);
                    strcpy(display_state, "NORM"); 
                } 
                else if (strcmp(current_state_str, "CRITICAL") == 0) 
                {
                    UI_PORT |= (1<<LED_RED);
                    strcpy(display_state, "CRIT"); 
                } 
                else 
                {
                    UI_PORT |= (1<<LED_YELLOW);
                    if (strcmp(current_state_str, "MEDIUM") == 0) strcpy(display_state, "MED");
                    else strcpy(display_state, current_state_str); 
                }

                lcd_clear();
                sprintf(lcd_buf, "TEMP: %d C %s", current_temp, is_auto ? "AUTO" : "MAN");
                lcd_puts(lcd_buf);
                
                lcd_gotoxy(0, 1);
                if (current_fan == 100) sprintf(lcd_buf, "FAN :%d%% %s", current_fan, display_state);
                else sprintf(lcd_buf, "FAN : %d%% %s", current_fan, display_state);
                lcd_puts(lcd_buf);
            }

            if (strcmp(current_state_str, "CRITICAL") == 0)
            {
                if (buzzer_timer >= 500)
                {
                    buzzer_timer = 0;
                    buzzer_state = !buzzer_state;
                    if (buzzer_state) UI_PORT |= (1<<BUZZER_PIN);
                    else UI_PORT &= ~(1<<BUZZER_PIN);
                }
            }
            else
            {
                UI_PORT &= ~(1<<BUZZER_PIN);
            }
        }
    }
}