#include "../include/config.h"
#include "../include/adc.h"

void adc_init(void)
{
    ADMUX = 0x40;  // AVCC reference
    ADCSRA = 0x86; // Enable ADC, Prescaler = 64
}

unsigned int read_adc(unsigned char adc_input)
{
    ADMUX = adc_input | 0x40; 
    
    #asm
        nop
        nop
        nop
        nop
    #endasm
    
    ADCSRA |= 0x40;           
    while ((ADCSRA & 0x10) == 0); 
    ADCSRA |= 0x10;           
    return ADCW;
}