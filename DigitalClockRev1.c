#include <avr/io.h>
#include <avr/interrupt.h>
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
	//todo configure external interrupts

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

ISR(TIMER2_OVF_vect)
{
	global_seconds++;
	if( global_seconds > 59 )
	{
		global_seconds = 0;
		global_minutes++;
	}
	if( global_minutes > 59 )
	{
		global_minutes = 0;
		global_hours++;
	}
	if( global_hours > 24 )
	{
		global_hours = 0;
	} 	

	updateDisplay( global_minutes, global_seconds );

	// todo: go to sleep
}

void ISR_buttonPress (void)
{
	// check which button
	// increment time
}
