/*
 * clock.c
 *
 * Created: 6/28/2011 2:44:40 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---LOCAL HEADER FILES--- */
#include "clock.h"

// TODO: turn off unused oscillators after switching

/* ---FUNCTION DEFINITIONS--- */
void clock_set_2mhz_internal(void)
{
	OSC.CTRL |= 0x01;				// enable 2MHz internal oscillator
	while(!(OSC.STATUS & 0x01));	// wait for oscillator stability 
	CCP = 0xD8;						// allow changing of protected register
	CLK.CTRL = 0x00;				// select 2 MHz internal RC oscillator
}

void clock_set_32mhz_internal(void)
{
	OSC.CTRL |= 0x02;				// enable 32MHz internal oscillator
	while(!(OSC.STATUS & 0x02));	// wait for oscillator stability 
	CCP = 0xD8;						// allow changing of protected register
	CLK.CTRL = 0x01;				// select 32 MHz internal RC oscillator
}

void clock_set_xosc(void)
{
	OSC.XOSCCTRL = 0b11000111;		// setup for 16 MHz oscillator with 1k startup time
	OSC.CTRL |= 0b00001000;			// enable XOSC
	while(!(OSC.STATUS & 0x08));	// wait for oscillator stability
	CCP = 0xD8;						// allow changing of protected register
	CLK.CTRL = 0x03;				// select XOSC as clock
}

void clock_set_32mhz_crystal(void)
{
	OSC.XOSCCTRL = 0b11000111;		// setup for 16 MHz oscillator with 1k startup time
	OSC.CTRL |= 0b00001000;			// enable XOSC
	OSC.PLLCTRL = 0b11000010;		// setup PLL with XOSC as source and 2x multiplier
	while(!(OSC.STATUS & 0x08));	// wait for oscillator stability
	OSC.CTRL |= 0b00010000;			// enable PLL
	while(!(OSC.STATUS & 0x10));	// wait for PLL stability
	CCP = 0xD8;						// allow changing of protected register
	CLK.CTRL = 0x04;				// select PLL as clock
}

void clock_delay(char num)
{
	for (num; num > 0; num--) nop();
}