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
// Calculate the percent change between two floats
float percent_change(float number, float reference)
{
	return((number - reference) / reference);
}

// Calculate the difference between two floats, ensuring that the results is between pi and -pi
float calculate_error(float desired, float actual)
{
	float error;
	
	error = desired - actual;
	if (error <= -M_PI) error += 2*M_PI;
	else if (error > M_PI) error -= 2*M_PI;
	
	return(error);
}

// Check if a number is between -pi and pi, if is isn't add multiple to 2pi to make it so
float valid_angle(float angle)
{
	while (angle <= -M_PI) angle += 2*M_PI;
	while (angle > M_PI) angle -= 2*M_PI;
	
	return(angle);
}