#include <avr/io.h>
#include <util/delay_basic.h>
#include "Display.h"
#include "DigitalClockRev1.h"

void SPI16write (uint16_t data)
{
	uint8_t count;

	for (count = 16; count > 0; count--)
	{
		if (data & 1)
		{
			disp_data_port |= _BV(disp_data_bit);
		}
		else
		{
			disp_data_port &= ~_BV(disp_data_bit);
		}
		_delay_loop_1(50);
		disp_clk_port |= _BV(disp_clk_bit);
		_delay_loop_1(50);
		disp_clk_port &= ~_BV(disp_clk_bit);
		data = data >> 1;
	}
}

uint32_t timeToBitString (volatile uint8_t hour, volatile uint8_t minute)
{
							// 0babcdefgp
	uint8_t charMap[] = {	0xFC,
							0x60,
							0xDA,
							0xF2,
							0x66,
							0xB6,
							0xBE,
							0xE0,
							0xFE,
							0xF6
						};

	//returns	    hr_tens    hr_ones   min_tens    min_ones
	//          msb 0babcdefgp 0babcdefgp 0babcdefgp 0babcdefgp  lsb
	return ((uint32_t)charMap[hour / 10] << 24) | ((uint32_t)charMap[hour % 10] << 16) | ((uint32_t)charMap[minute / 10] << 8) | (uint32_t)charMap[minute % 10];
}


uint32_t shuffleBuffer (uint32_t displayBits )
{
	uint32_t shuffledBuffer = 0;
	uint8_t i;
								//		a, b, c, d, e, f, g, p
	uint8_t	shuffleMap[] =	{	20, 24, 25, 23, 22, 21, 27, 26,
								16, 28, 29, 19, 18, 17, 31, 30,
								12,  0,  1, 15, 14, 13,  3,  2,
								 8,  4,  5, 11, 10,  9,  7,  6
							};
	
	for( i = 0; i < 32; i++ )
	{
		if( displayBits & 1 )
			shuffledBuffer |= ((uint32_t)1 << shuffleMap[i]);
		displayBits >>= 1;
	}

	return shuffledBuffer;
}

void updateDisplay ( uint8_t hour, uint8_t minute )
{
	uint32_t displayBuffer;
	uint32_t unshuffled;
	
	unshuffled = timeToBitString( hour, minute );
	displayBuffer = shuffleBuffer(unshuffled);

//	displayBuffer = shuffleBuffer(timeToBitString( hour, minute ));
	disp_latch_port &= ~_BV(disp_latch_bit);

	SPI16write((uint16_t)(displayBuffer & 0x0000FFFF));
	SPI16write(displayBuffer >> 16);

	disp_latch_port |= _BV(disp_latch_bit);
}
