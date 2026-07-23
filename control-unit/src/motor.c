#include "../include/config.h"
#include "../include/motor.h"

void motor_init(void)
{
    MOTOR_DDR = (1<<MOTOR_PWM) | (1<<MOTOR_IN1) | (1<<MOTOR_IN2);
    MOTOR_PORT = (1<<MOTOR_IN1); 

    // Timer 1: Fast PWM 8-bit, non-inverted, prescaler 64
    TCCR1A = (1<<COM1A1) | (1<<WGM10);
    TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
}

void motor_set_speed(unsigned int percent)
{
    OCR1A = (unsigned int)((percent * 255.0) / 100.0);
}