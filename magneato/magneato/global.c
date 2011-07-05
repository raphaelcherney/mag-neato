/*
 * global.c
 *
 * Created: 7/5/2011 12:37:44 PM
 *  Author: Raphael
 */ 

/* ---LOCAL HEADER FILES--- */
#include "global.h"

/* ---FUNCTION DEFINITIONS--- */
float percent_change(float number, float reference)
{
	return((number - reference) / reference);
}