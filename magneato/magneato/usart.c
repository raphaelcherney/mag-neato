/*
 * usart.c
 *
 * Created: 6/30/2011 2:23:23 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---FUNCTION DEFINITIONS--- */
void usart_init(void)
{
	PORTC.OUTSET = 0b00001000;		// set TXD0 pin (PC3) high
	PORTC.DIRSET = 0b00001000;		// set PC3 as output
	USARTC0.BAUDCTRLA = 207;		// set baudrate to 9600
	USARTC0.CTRLB = 0b00001000;		// enable transmitter
}

void usart_transmit_char(unsigned char data)
{
	while (!(USARTC0.STATUS & 0b00100000));	// wait until transmit buffer is ready to receive new data
	USARTC0.DATA = data;					// send the data
}

void usart_transmit_string(const char *str)
{
	while (*str)
	{
		usart_transmit_char(*str);
		str++;
	}
}