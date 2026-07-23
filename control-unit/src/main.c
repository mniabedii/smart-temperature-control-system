#include <stdio.h>
#include <string.h>
#include "../include/config.h"
#include "../include/adc.h"
#include "../include/motor.h"
#include "../include/uart.h"

// System variables
volatile unsigned int adc_timer = 0;
volatile unsigned int uart_timer = 0;
float current_temp = 0.0;
unsigned int fan_speed_percent = 0;

// UART buffers
char tx_buffer[40]; 
volatile char rx_buffer[20];
volatile unsigned char rx_index = 0;
volatile unsigned char cmd_ready = 0;

// Manual mode variables
unsigned char is_auto = 1;
unsigned int manual_fan_speed = 0;

typedef enum {
    NORMAL = 0, LOW_SPEED, MEDIUM_SPEED, HIGH_SPEED, CRITICAL
} system_state_t;

system_state_t current_state = NORMAL;

// Timer 0 ISR (1ms)
interrupt [TIM0_COMP] void timer0_comp_isr(void)
{
    adc_timer++;
    uart_timer++;
}

// USART0 RX ISR
interrupt [USART0_RXC] void usart0_rx_isr(void)
{
    char data = UDR0;
    if (data == '\n')
    {
        rx_buffer[rx_index] = '\0'; 
        cmd_ready = 1;
        rx_index = 0; 
    }
    else
    {
        if (rx_index < 19) rx_buffer[rx_index++] = data;
    }
}

void update_fan_control(void)
{
    if (current_state == NORMAL) 
    {
        if (current_temp >= 25.0) current_state = LOW_SPEED;
    } 
    else if (current_state == LOW_SPEED) 
    {
        if (current_temp < 24.0) current_state = NORMAL;
        else if (current_temp >= 30.0) current_state = MEDIUM_SPEED;
    } 
    else if (current_state == MEDIUM_SPEED) 
    {
        if (current_temp < 29.0) current_state = LOW_SPEED;
        else if (current_temp >= 35.0) current_state = HIGH_SPEED;
    } 
    else if (current_state == HIGH_SPEED) 
    {
        if (current_temp < 34.0) current_state = MEDIUM_SPEED;
        else if (current_temp >= 40.0) current_state = CRITICAL;
    } 
    else if (current_state == CRITICAL) 
    {
        if (current_temp < 39.0) current_state = HIGH_SPEED;
    }

    switch (current_state) 
    {
        case NORMAL:       fan_speed_percent = 0;   break;
        case LOW_SPEED:    fan_speed_percent = 30;  break;
        case MEDIUM_SPEED: fan_speed_percent = 50;  break;
        case HIGH_SPEED:   fan_speed_percent = 75;  break;
        case CRITICAL:     fan_speed_percent = 100; break;
    }
}

void main(void)
{
    adc_init();
    motor_init();
    uart_init();

    // Timer 0 configuration
    TCCR0 = 0x0B; 
    OCR0 = 124;
    TIMSK = 0x02; 
    
    #asm("sei")

    while (1)
    {
        if (cmd_ready)
        {
            cmd_ready = 0; 
            if (rx_buffer[0] == 'A') is_auto = 1;
            else if (rx_buffer[0] == 'M') 
            {
                is_auto = 0;
                sscanf((char*)rx_buffer, "M:%d", &manual_fan_speed);
            }
        }

        if (adc_timer >= 100)
        {
            adc_timer = 0; 
            current_temp = (read_adc(0) * 500.0) / 1023.0;
            update_fan_control();
            
            if (!is_auto) fan_speed_percent = manual_fan_speed;
            motor_set_speed(fan_speed_percent);
        }
        
        if (uart_timer >= 1000)
        { 
            char current_state_str[15];
            uart_timer = 0; 
            
            switch (current_state) 
            {
                case NORMAL:       strcpy(current_state_str, "NORMAL");   break;
                case LOW_SPEED:    strcpy(current_state_str, "LOW");      break;
                case MEDIUM_SPEED: strcpy(current_state_str, "MEDIUM");   break;
                case HIGH_SPEED:   strcpy(current_state_str, "HIGH");     break;
                case CRITICAL:     strcpy(current_state_str, "CRITICAL"); break;
            }
            
            sprintf(tx_buffer, "T:%d,F:%d,S:%s\r\n", (int)current_temp, fan_speed_percent, current_state_str);
            uart_putstring(tx_buffer);
        }
    }
}