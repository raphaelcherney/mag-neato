/*
 * led.c
 *
 * Created: 6/28/2011 2:47:34 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"

/* ---FUNCTION DEFINITIONS--- */
void led_set(char led)
{
	switch (led)
	{
		case RED:
			PORTE.OUTSET = 0b10000000;
			break;
		case YELLOW:
			PORTE.OUTSET = 0b01000000;
			break;
		case GREEN:
			PORTE.OUTSET = 0b00100000;
			break;
		case BLUE:
			PORTE.OUTSET = 0b00010000;
			break;
		case UNDER:
			PORTE.OUTSET = 0b00001000;
			break;
		case ALL:
			PORTE.OUTSET = 0b11111000;
			break;
	}	
}

void led_toggle(char led)
{
	switch (led)
	{
		case RED:
			PORTE.OUTTGL = 0b10000000;
			break;
		case YELLOW:
			PORTE.OUTTGL = 0b01000000;
			break;
		case GREEN:
			PORTE.OUTTGL = 0b00100000;
			break;
		case BLUE:
			PORTE.OUTTGL = 0b00010000;
			break;
		case UNDER:
			PORTE.OUTTGL = 0b00001000;
			break;
		case ALL:
			PORTE.OUTTGL = 0b11111000;
			break;
	}
}

void led_clear(char led)
{
	switch (led)
	{
		case RED:
			PORTE.OUTCLR = 0b10000000;
			break;
		case YELLOW:
			PORTE.OUTCLR = 0b01000000;
			break;
		case GREEN:
			PORTE.OUTCLR = 0b00100000;
			break;
		case BLUE:
			PORTE.OUTCLR = 0b00010000;
			break;
		case UNDER:
			PORTE.OUTCLR = 0b00001000;
			break;
		case ALL:
			PORTE.OUTCLR = 0b11111000;
			break;
	}	
}