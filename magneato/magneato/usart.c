/*
 * usart.c
 *
 * Created: 6/30/2011 2:23:23 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>
#include <stdlib.h>

/* ---LOCAL HEADER FILES--- */
#include "usart.h"

/* ---FUNCTION DEFINITIONS--- */
// Initialize USART communications at 9600 baud
void usart_init(void)
{
	PORTC.OUTSET = 0b00001000;		// set TXD0 pin (PC3) high
	PORTC.DIRSET = 0b00001000;		// set PC3 as output
	USARTC0.BAUDCTRLA = 207;		// set baudrate to 9600
	USARTC0.CTRLB = 0b00001000;		// enable transmitter
}

// Transmit a character over USART
void usart_transmit_char(unsigned char data)
{
	while (!(USARTC0.STATUS & 0b00100000));	// wait until transmit buffer is ready to receive new data
	USARTC0.DATA = data;					// send the data
}

// Transmit a string representation of an integer over USART
void usart_transmit_int(int data)
{
	char string[6];
	
	itoa(data, string, 10);
	usart_transmit_string(string);
}

// Transmit a string representation of an unsigned integer over USART
void usart_transmit_unsigned_int(unsigned int data)
{
	char string[6];
	
	utoa(data, string, 10);
	usart_transmit_string(string);
}

// Transmit a string representation of an float over USART
void usart_transmit_float(float data)
{
	char string[10];
	
	dtostrf(data, 8, 6, string);
	usart_transmit_string(string);
}

// Transmit a string over USART
void usart_transmit_string(const char *str)
{
	while (*str)
	{
		usart_transmit_char(*str);
		str++;
	}
}