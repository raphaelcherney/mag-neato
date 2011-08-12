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
// Use the 2MHz internal RC oscillator as the clock source
void clock_set_2mhz_internal(void)
{
	OSC.CTRL |= 0x01;				// enable 2MHz internal oscillator
	while(!(OSC.STATUS & 0x01));	// wait for oscillator stability 
	CCP = 0xD8;						// allow changing of protected register
	CLK.CTRL = 0x00;				// select 2 MHz internal RC oscillator
}

// Use the 32MHz internal RC oscillator as the clock source
void clock_set_32mhz_internal(void)
{
	OSC.CTRL |= 0x02;				// enable 32MHz internal oscillator
	while(!(OSC.STATUS & 0x02));	// wait for oscillator stability 
	CCP = 0xD8;						// allow changing of protected register
	CLK.CTRL = 0x01;				// select 32 MHz internal RC oscillator
}

// Use a 16 MHz external oscillator along with a 2x PLL as the clock source
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

// Simple clock delay function (usually better to use library functions instead: _delay_ms(5);)
void clock_delay(volatile char num)
{
	for (num; num > 0; num--) nop();
}