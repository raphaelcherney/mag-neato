/*
 * ir.c
 *
 * Created: 7/5/2011 10:30:44 AM
 *  Author: Raphael
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---FUNCTION DEFINITIONS--- */
void ir_enable(void)
{
	PORTA.OUTSET = 0b00000001;
}

void ir_disable(void)
{
	PORTA.OUTCLR = 0b00000001;
}

void ir_set_threshold(char level)
{
	ACA.CTRLB = level;	// set scaled VCC level
}

void ir_change_threshold(signed char amount)
{
	char level;
	
	level = ACA.CTRLB;
	level = level + amount;
	ACA.CTRLB = level;
}