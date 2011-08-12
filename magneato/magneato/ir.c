/*
 * ir.c
 *
 * Created: 7/5/2011 10:30:44 AM
 *  Author: Raphael
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---FUNCTION DEFINITIONS--- */
// Initialize the IR sensors
void ir_init(void)
{
	ACA.AC0MUXCTRL = 0b00101111;	// use PA5 and scaled VCC for ACA0
	ACA.AC1MUXCTRL = 0b00110111;	// use PA6 and scaled VCC for ACA1
	ACA.CTRLB = 63;					// set initial scaled VCC level
	//ACA.AC0CTRL = 0b11101101;		// trigger MID level interrupt on rising edge using high-speed mode and large hysteresis
	//ACA.AC1CTRL = 0b11101101;		// enable ACA1 with same settings
}

// Enable the reflective IR sensors
void ir_enable(void)
{
	PORTA.OUTSET = 0b00000001;
}

// Disable the reflective IR sensors
void ir_disable(void)
{
	PORTA.OUTCLR = 0b00000001;
}

// Set the threshold for causing an interrupt
void ir_set_threshold(char level)
{
	ACA.CTRLB = level;	// set scaled VCC level
}

// Change the interrupt threshold
void ir_change_threshold(signed char amount)
{
	char level;
	
	level = ACA.CTRLB;
	level = level + amount;
	ACA.CTRLB = level;
}