#define F_CPU 1000000UL // 1 MHz
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include "DigitalClockRev1.h"
#include "Display.h"

void initialize_display(void)
{
	//configure 16-bit timer register
//    Fast PWM Mode - TOP=OCR1A
//    Set OC1B on compare match, clear OC1B at BOTTOM
//    Clock: clk_I/O (no prescale)
    TCCR1A = _BV(COM1B1) | _BV(COM1B0) | _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
    OCR1A = 5000; // Reset PWM when counter reaches 5000, should be 200Hz.
    OCR1B = 2500;

	disp_OE_port &= ~_BV(disp_OE_bit); // enable display
}

void disable_display(void)
{
    TCCR1B &= ~_BV(CS12) & ~_BV(CS11) & ~_BV(CS10); // Set Timer1 clock source to none
    disp_OE_port |= _BV(disp_OE_bit); // disable display
}

void disable_buttons(void)
{
    PCICR = 0; // Button interrupts disabled
}

void enable_buttons(void)
{
    PCIFR |= _BV(PCIF0) | _BV(PCIF1) | _BV(PCIF2); // clear interrupt flags if they may have occurred
    PCICR = _BV(PCIE0) | _BV(PCIE1) | _BV(PCIE2);
}

void initialize_buttons(void)
{
	// configure external interrupts
    // B1 = Button 4 = PCINT1
    // C2 = Button 2 = PCINT10
    // D5 = Button 1 = PCINT21
    // D6 = Button 3 = PCINT22

    PCMSK0 = _BV(PCINT1);
    PCMSK1 = _BV(PCINT10);
    PCMSK2 = _BV(PCINT21) | _BV(PCINT22);
    PCICR = _BV(PCIE0) | _BV(PCIE1) | _BV(PCIE2);
}

void initialize(void)
{
	//configure IO
	DDRB = 0b00000100;
	DDRC = 0b00011010;
	DDRD = 0;

	PORTB = 0b11111111;
	PORTC = 0b01000101;
	PORTD = 0b01111111;

	//todo set power reduction register (PRR)

	//configure asynchronous timer registers
	TCCR2A = 0;
	TCCR2B = _BV(CS22) | _BV(CS20); // set prescaler to 128 (32,768 Hz / 128 = 256 counts / sec)
	TIMSK2 = _BV(TOIE2); // enable Timer/Counter2 Overflow interrupt (TOV2 bit set in TIFR2)
	ASSR = _BV(AS2); // clock Timer/Counter2 from a crystal Oscillator

	//configure analog comparator
    // Analog comparator multiplexer disabled (AIN1 applied to negative input)
    // AC bandgap selected (takes ~70us to stabilize)
    // AC interrupt enabled on output toggle
    ADCSRB = 0;
    ACSR = _BV(ACBG);
    DIDR1 = _BV(AIN1D); // Disable digital pin AIN1

    _delay_ms(1); // give time for analog voltage and timer to stabilize
    ACSR |= _BV(ACIE); // Enable AC interrupt
}

volatile uint8_t global_hours = 0;
volatile uint8_t global_minutes = 0;
volatile uint8_t global_seconds = 0;
volatile uint8_t known_time = 0;

// wake-up flags
volatile uint8_t button_pressed = 0;
volatile uint8_t second_passed = 0;
volatile uint8_t power_change = 0;

void handle_power(void)
{
    _delay_us(8); // wait to synchronize register with AC output
    if(!(ACSR & _BV(ACO)))
    {
        // We are plugged in
        initialize_display();
        enable_buttons();
    }
    else
    {
        // We are on battery power, turn off display and buttons
        disable_display();
        disable_buttons();
    }
}

void handle_button(void)
{
    disable_buttons();
    _delay_ms(10);

    if(!(button_1_pin&_BV(button_1_bit))) // Button 1 (Right-Up)
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
    else if(!(button_3_pin&_BV(button_3_bit))) // Button 3 (Right-Down)
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
    else if(!(button_2_pin&_BV(button_2_bit))) // Button 2 (Left-Up)
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
    else if(!(button_4_pin&_BV(button_4_bit))) // Button 4 (Left-down)
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
    enable_buttons();
    known_time = 1;
}

void increment_time(void)
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

    // Dim during night hours
    if( global_hours < 5 || global_hours > 21 )
    {
        OCR1B = 1000;
    }
    else
    {
        OCR1B = 5000;
    }
 // if the time is reset, flash display
    if((known_time == 0) && (global_seconds % 2))
    {
        OCR1B = 1;
    }
}

int main(void)
{
	initialize();

    sei();
    if(!(ACSR & _BV(ACO)))
    {
        // We are plugged in
        initialize_display();
        initialize_buttons();
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
    }
    else
    {
        // We are on battery power, go to sleep without display or buttons
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        sleep_mode();
    }
    initialize_display();
    initialize_buttons();
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_mode();

	while(1)
    {
        // something woke us up
        if(second_passed)
        {
            increment_time();
            second_passed = 0;
        }
        if(button_pressed)
        {
            handle_button();
            button_pressed = 0;
        }
        if(power_change)
        {
            handle_power();
            power_change = 0;
        }
        
        if(!(ACSR & _BV(ACO)))
        {
            // plugged in
            updateDisplay( global_hours, global_minutes );
            set_sleep_mode(SLEEP_MODE_IDLE);
            sleep_mode();
        }
        else
        {
            set_sleep_mode(SLEEP_MODE_PWR_SAVE);
            sleep_mode();
        }
    }
	return 0;
}

ISR(ANALOG_COMP_vect)
{
    power_change = 1;
}

ISR(TIMER2_OVF_vect)
{
    second_passed = 1;
}

ISR(PCINT0_vect)
{
    button_pressed = 1;
}

ISR(PCINT1_vect)
{
    button_pressed = 1;
}

ISR(PCINT2_vect)
{
    button_pressed = 1;
}
