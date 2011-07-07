/*
 * global.c
 *
 * Created: 7/5/2011 12:37:44 PM
 *  Author: Raphael
 */ 

/* ---AVR HEADER FILES--- */
#include <math.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"

/* ---FUNCTION DEFINITIONS--- */
float percent_change(float number, float reference)
{
	return((number - reference) / reference);
}

float calculate_error(float desired, float actual)
{
	float error;
	
	error = desired - actual;
	if (error < -M_PI) error += 2*M_PI;
	else if (error > M_PI) error -= 2*M_PI;
	
	return(error);
}