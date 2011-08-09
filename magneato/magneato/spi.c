/*
 * spi.c
 *
 * Created: 8/9/2011 2:35:19 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "spi.h"

/* ---FUNCTION DEFINITIONS--- */
void spi_enable(void)
{
	PORTD.OUTSET = 0b00000010;	// set CS high
	PORTC.DIRSET = 0b10110010;	// set PC1 (CS), PC4 (SS), PC5 (MOSI), PC7 (SCK) as outputs
	PORTC.DIRCLR = 0b01000000;	// set PC6 (MISO) as input
	SPIC.CTRL = 0b01011110;		// enable SPI master in Mode 3 with prescaler:clk/64 (500 kHz)
	//cleaner:
	//SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_3_gc | SPI_PRESCALER_DIV64_gc;
}

void spi_write(unsigned char data)
{
	PORTC.OUTCLR = 0b00000010;					// set CS low
	SPIC.DATA = data;							// transmit data
	while (!bit_check(SPIC.STATUS, BIT(7)));	// wait for transmission to end
	PORTC.OUTSET = 0b00000010;					// set CS high
}

char spi_read(void)
{
	spi_write(0);			// send empty data to run clock
	return SPIC.DATA;		// read out data
}

// TODO: call "spi_transfer" instead, add "spi_message"