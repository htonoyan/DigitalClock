#define F_CPU 1000000UL // 1 MHz
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "DigitalClockRev1.h"
#include "Display.h"

void initialize (void)
{
	//configure IO
	DDRB = 0b00000100;
	DDRC = 0b00011010;
	DDRD = 0;
	
	PORTB = 0b11111111;
	PORTC = 0b01000101;
	PORTD = 0b01111111;

	//todo set power reduction register (PRR)

	// configure external interrupts
    // B1 = Button 4 = PCINT1
    // C2 = Button 2 = PCINT10
    // D5 = Button 1 = PCINT21
    // D6 = Button 3 = PCINT22
    
    PCMSK0 = _BV(PCINT1) | _BV(PCINT2);
    PCMSK1 = _BV(PCINT10);
    PCMSK2 = _BV(PCINT21) | _BV(PCINT22);
    PCICR = _BV(PCIE0) | _BV(PCIE1) | _BV(PCIE2);

	//configure asynchronous timer registers
	TCCR2A = 0;
	TCCR2B = _BV(CS22) | _BV(CS20); // set prescaler to 128 (32,768 Hz / 128 = 256 counts / sec)
	TIMSK2 = _BV(TOIE2); // enable Timer/Counter2 Overflow interrupt (TOV2 bit set in TIFR2)
	ASSR = _BV(AS2); // clock Timer/Counter2 from a crystal Oscillator
	
	//configure 16-bit timer register

	//configure analog comparator
	
	//configure ADC
	
	//todo set interrupts

}

volatile uint8_t global_hours = 0;
volatile uint8_t global_minutes = 0;
volatile uint8_t global_seconds = 0;

int main (void)
{
//	volatile uint8_t testHour = 10;
//	volatile uint8_t testMinute = 49;

//	volatile uint32_t testTime;
	initialize();
	sei(); // enable global interrupts
	
	disp_OE_port &= ~_BV(disp_OE_bit); // enable display
	//todo: sleep
	while(1);
	return 0;
}

void ISR_buttonPress(void)
{
    PCICR = 0; // Disable pin change interrupts to avoid bouncing causing issues
    _delay_ms(10);

    if(!(PIND&_BV(5))) // Button 1 (Right-Up)
    {
        if(global_minutes != 59)
        {
            global_minutes++;
        }
        else
        {
            global_minutes = 0;
        }
        global_seconds = 0;
        TCNT2 = 0;
    }
    else if(!(PIND&_BV(6))) // Button 3 (Right-Down)
    {
        if(global_minutes != 0)
        {
            global_minutes--;
        }
        else
        {
            global_minutes = 59;
        }
        global_seconds = 0;
        TCNT2 = 0;
    }
    else if(!(PINC&_BV(2))) // Button 2 (Left-Up)
    {
        if(global_hours != 23)
        {
            global_hours++;
        }
        else
        {
            global_hours = 0;
        }
    }
    else if(!(PINB&_BV(1))) // Button 4 (Left-down)
    {
        if(global_hours != 0)
        {
            global_hours--;
        }
        else
        {
            global_hours = 23;
        }
    }
    
    updateDisplay( global_hours, global_minutes );
    PCIFR |= _BV(PCIF0) | _BV(PCIF1) | _BV(PCIF2); // clear interrupt flags if they may have occured
    PCICR = _BV(PCIE0) | _BV(PCIE1) | _BV(PCIE2);
}

ISR(TIMER2_OVF_vect)
{
	global_seconds++;
	if( global_seconds > 59 )
	{
		global_seconds = 0;
		global_minutes++;
        updateDisplay( global_hours, global_minutes );
	}
	if( global_minutes > 59 )
	{
		global_minutes = 0;
		global_hours++;
        updateDisplay( global_hours, global_minutes );
        
	}
	if( global_hours > 24 )
	{
		global_hours = 0;
        updateDisplay( global_hours, global_minutes );
	}
    

	// todo: go to sleep
}

ISR(PCINT0_vect)
{
    sei();
    ISR_buttonPress();
}

ISR(PCINT1_vect)
{
    sei();
    ISR_buttonPress();
}

ISR(PCINT2_vect)
{
    sei();
    ISR_buttonPress();
}
