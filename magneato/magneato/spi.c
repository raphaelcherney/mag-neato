/*
 * spi.c
 *
 * Created: 8/9/2011 2:35:19 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>
#define F_CPU 32000000UL		// 32 MHz
#include <util/delay.h>			// delay functions

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "spi.h"

/* ---FUNCTION DEFINITIONS--- */
// Enable SPI communication through the expansion port and begin configuring the ADXL345
void spi_enable(void)
{
	char out[1];
	
	PORTC.OUTSET = 0b00000010;	// set CS high
	PORTC.DIRSET = 0b10110010;	// set PC1 (CS), PC4 (SS), PC5 (MOSI), PC7 (SCK) as outputs
	PORTC.DIRCLR = 0b01000000;	// set PC6 (MISO) as input
	SPIC.CTRL = 0b01011111;		// enable SPI master in Mode 3 with prescaler:clk/64 (500 kHz)
	//cleaner:
	//SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_3_gc | SPI_PRESCALER_DIV64_gc;
	
	while (out[0] != 0x08)
	{
		spi_write_register(0x2D, 0x08);	// turn on measurement
		_delay_ms(5);
		spi_read_register(0xAD, 1, out);	// read out that register
		_delay_ms(1000);
	}
	
	spi_write_register(0x31, 0x08);	// set full range
}

// Transmit a character over SPI and wait until the transmission finishes
void spi_transmit(unsigned char data)
{
	char temp = SPIC.STATUS;					// clear the interrupt flag
	SPIC.DATA = data;							// transmit data
	while (!bit_check(SPIC.STATUS, BIT(7)));	// wait for transmission to end
}

// Run the clock and read the next character
char spi_read(void)
{
	spi_transmit(0);		// send empty data to run clock
	return SPIC.DATA;		// read out data
}

// Write to the ADXL345 registers (two-byte SPI messages)
void spi_write_register(char register_address, char data)
{
  PORTC.OUTCLR = 0b00000010;		// set CS pin low to signal the beginning of an SPI packet
  _delay_us(5);
  spi_transmit(register_address);	// transfer the register address over SPI.
  spi_transmit(data);				// transfer the desired register value over SPI.
  _delay_us(5);
  PORTC.OUTSET = 0b00000010;		// set the CS pin high to signal the end of SPI packet
}

// Read out num_bytes worth of information starting with the register at register_address to the array data.
void spi_read_register(char register_address, int num_bytes, char *data)
{
	char address = 0x80 | register_address;			// since we're performing a read operation, the most significant bit of the register address should be set
	if (num_bytes > 1) address = address | 0x40;	// if we're doing a multi-byte read, bit 6 needs to be set as well
	
	PORTC.OUTCLR = 0b00000010;		// set CS pin low
	spi_transmit(address);			// transfer the register address that needs to be read
	for (int i=0; i<num_bytes; i++)
	{
		data[i] = spi_read();		// continue reading registers until we read the number specified
	}
	PORTC.OUTSET = 0b00000010;		// set the CS pin high
}