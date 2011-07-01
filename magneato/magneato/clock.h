/*
 * clock.h
 *
 * Created: 6/28/2011 2:44:52 PM
 *  Author: Raphael
 */ 


#ifndef CLOCK_H
#define CLOCK_H

/* ---DEFINE MACROS--- */
#define nop()	asm("nop")

/* ---FUNCTION PROTOTYPES--- */
void clock_set_2mhz_internal(void);
void clock_set_32mhz_internal(void);
void clock_set_xosc(void);
void clock_set_32mhz_crystal(void);
void clock_delay(char num);

#endif /* CLOCK_H */