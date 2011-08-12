/*
 * color.c
 *
 * Created: 6/29/2011 11:07:17 AM
 *  Author: Raphael
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>
#include <stdlib.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "color.h"
#include "usart.h"
#include "led.h"
#include "motor.h"

/* ---GLOBAL VARIABLES--- */
volatile unsigned int global_color_value[6][4];
volatile unsigned int global_color_calibrate[6][4];
volatile float global_color_change[6][4];
volatile char global_color_sensor_count;
volatile char global_color_filter;
extern volatile char global_state;
volatile float global_color_threshold[4];
extern volatile float global_desired_angle;

/* ---FUNCTION DEFINITIONS--- */
// Set the filter to use with the color sensors.  Choices are: RED, BLUE, CLEAR, or GREEN.
void color_set_filter(char color)
{
	switch (color)
	{
		case RED:
			PORTF.OUTCLR = 0b01100000;
			break;
		case BLUE:
			PORTF.OUTCLR = 0b00100000;
			PORTF.OUTSET = 0b01000000;
			break;
		case CLEAR:
			PORTF.OUTCLR = 0b01000000;
			PORTF.OUTSET = 0b00100000;
			break;
		case GREEN:
			PORTF.OUTSET = 0b01100000;
			break;
	}	
}

// Set the frequency scale you want the color sensors to output.  Choices are: TWO_PERCENT, TWENTY_PERCENT, ONE_HUNDRED_PERCENT, or OFF.
void color_set_frequency(char scale)
{
	switch (scale)
	{
		case OFF:
			PORTF.OUTCLR = 0b00011000;
			break;
		case TWO_PERCENT:
			PORTF.OUTCLR = 0b00001000;
			PORTF.OUTSET = 0b00010000;
			break;
		case TWENTY_PERCENT:
			PORTF.OUTCLR = 0b00010000;
			PORTF.OUTSET = 0b00001000;
			break;
		case ONE_HUNDRED_PERCENT:
			PORTF.OUTSET = 0b00011000;
			break;
	}	
}

// Initialize the color sensors to take a reading every 1ms.  TCE1 is used to trigger the reading and update which sensor should be measured.
//  TCE0 is used to take the actual measurement based on CH0 input
void color_init(void)
{
	color_set_frequency(ONE_HUNDRED_PERCENT);
	
	TCE1.CTRLA = 0b00000001;		// enable TCE1 with prescaler: clk/1 (will overflow every 2ms)
	//TCE1.INTCTRLA = 0b00000001;		// configure timer overflow as LOW level interrupt
	TCE1.CCA = 32767;				// trigger CCA every 1 ms
	TCE1.INTCTRLB = 0b00000001;		// configure CCA as LOW level interrupt
	
	TCE0.CTRLD = 0b10101000;		// frequency capture using event CH0
	TCE0.CTRLB |= 0b00010000;		// enable CCA
	TCE0.CTRLA = 0b00000001;		// enable TCE0 with prescaler: clk/1
	
	EVSYS.CH0MUX = 0b01110000;		// set PE0 as CH0 event trigger (assign starting state)
	global_color_sensor_count = 0;	// measuring sensor 0
	color_set_filter(CLEAR);
	global_color_filter = 0;
}

// Update which color is being sensed (function is called many times per second)
void color_update(void)
{
	switch (global_color_sensor_count)
	{
		case 0:
			global_color_value[0][global_color_filter] = TCE0.CCA;	// read out sensor 0 frequency
			EVSYS.CH0MUX = 0b01110001;								// set PE1 as CH0 event trigger
			global_color_sensor_count = 1;							// measuring sensor 1...
			break;
		case 1:
			global_color_value[1][global_color_filter] = TCE0.CCA;	// read out sensor 1 frequency
			EVSYS.CH0MUX = 0b01110010;								// set PE2 as CH0 event trigger
			global_color_sensor_count = 2;							// measuring sensor 2...
			break;
		case 2:
			global_color_value[2][global_color_filter] = TCE0.CCA;	// read out sensor 2 frequency
			EVSYS.CH0MUX = 0b01111000;								// set PF0 as CH0 event trigger
			global_color_sensor_count = 3;							// measuring sensor 3...
			break;
		case 3:
			global_color_value[3][global_color_filter] = TCE0.CCA;	// read out sensor 3 frequency
			EVSYS.CH0MUX = 0b01111001;								// set PF1 as CH0 event trigger
			global_color_sensor_count = 4;							// measuring sensor 4...
			break;
		case 4:
			global_color_value[4][global_color_filter] = TCE0.CCA;	// read out sensor 4 frequency
			EVSYS.CH0MUX = 0b01111010;								// set PF2 as CH0 event trigger
			global_color_sensor_count = 5;							// measuring sensor 5...
			break;
		case 5:
			global_color_value[5][global_color_filter] = TCE0.CCA;	// read out sensor 5 frequency
			EVSYS.CH0MUX = 0b01110000;								// set PE0 as CH0 event trigger
			global_color_sensor_count = 0;							// measuring sensor 0...
			switch (global_color_filter)							// change filter color
			{
				case 0:
					color_set_filter(RED);
					global_color_filter = 1;
					break;
				case 1:
					color_set_filter(GREEN);
					global_color_filter = 2;
					break;
				case 2:
					color_set_filter(BLUE);
					global_color_filter = 3;
					break;
				case 3:
					color_set_filter(CLEAR);
					global_color_filter = 0;
					color_change();
					//color_check_for_red();
					//color_transmit_value();
					break;
			}
			break;
	}
}

// A simple test function changing the states based on which sensors see lines
void color_compare(void)
{
	char i;
	
	for (i=0; i<4; i++)
			{
				if (global_color_change[0][i]>global_color_threshold[i])
				{
					global_state = HARD_LEFT;
				}
				else if (global_color_change[1][i]>global_color_threshold[i])
				{
					global_state = MID_LEFT;
					global_desired_angle = valid_angle(global_desired_angle + 0.66);
					led_set(BLUE);
				}
				else if (global_color_change[2][i]>global_color_threshold[i])
				{
					global_state = SLIGHT_LEFT;
					global_desired_angle = valid_angle(global_desired_angle + 0.25);
				}
				else if (global_color_change[3][i]>global_color_threshold[i])
				{
					global_state = SLIGHT_RIGHT;
					global_desired_angle = valid_angle(global_desired_angle - 0.25);
				}
				else if (global_color_change[4][i]>global_color_threshold[i])
				{
					global_state = MID_RIGHT;
					global_desired_angle = valid_angle(global_desired_angle - 0.66);
				}
				else if (global_color_change[5][i]>global_color_threshold[i])
				{
					global_state = HARD_RIGHT;
				}
			}
}

// Save the current sensor readings into the array global_color_calibrate
void color_calibrate(void)
{
	char i, j;
	
	for (i=0; i<6; i++)
	{
		for (j=0; j<4; j++)
		{
			global_color_calibrate[i][j] = global_color_value[i][j];
		}
	}
}

// Calculate the percent change between the current readings and the array global_color_calibrate and save it into global_color_change
void color_change(void)
{
	char i, j;
	
	for (i=0; i<6; i++)
	{
		for (j=0; j<4; j++)
		{
			global_color_change[i][j] = percent_change((float)global_color_value[i][j], (float)global_color_calibrate[i][j]);
		}
	}
}

// A simple function that checks if there is a stronger red signal than blue or green and reacts accordingly
// TODO: this function should be rewritten to be actually useful :-)
void color_check_for_red(void)
{
	int i, j;
	float threshold = 0.15;
	
	/*
	for (i=0; i<4; i++)
	{
		if (global_color_change[0][i]>threshold)
		{
			led_set(BLUE);
		}
		else if (global_color_change[1][i]>threshold)
		{
			led_set(BLUE);
		}
		else if (global_color_change[2][i]>threshold)
		{
			led_set(BLUE);
		}
		else if (global_color_change[3][i]>threshold)
		{
			led_set(GREEN);
		}
		else if (global_color_change[4][i]>threshold)
		{
			led_set(GREEN);
		}
		else if (global_color_change[5][i]>threshold)
		{
			led_set(GREEN);
		}
	}	
	*/
	for (i=0; i<2; i++)
	{
		if ((global_color_value[i][1]<global_color_value[i][2]) && (global_color_value[i][1]<global_color_value[i][3]))
		{
			global_state = REVERSE_LEFT;
			led_set(RED);
			break;
		}
	}
	for (j=3; j<5; j++)
	{
		if ((global_color_value[j][1]<global_color_value[j][2])  && (global_color_value[j][1]<global_color_value[j][3]))
		{
			global_state = REVERSE_RIGHT;
			led_set(RED);
			break;
		}
	}		
}

// Transmit the current color values over USART (can be read out through the terminal 9600 baud)
void color_transmit_value(void)
{
	char string[6];
	
	usart_transmit_string("S0:");
	usart_transmit_char(9);
	utoa(global_color_value[0][0], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[0][1], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[0][2], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[0][3], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S1:");
	usart_transmit_char(9);
	utoa(global_color_value[1][0], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[1][1], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[1][2], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[1][3], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S2:");
	usart_transmit_char(9);
	utoa(global_color_value[2][0], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[2][1], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[2][2], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[2][3], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S3:");
	usart_transmit_char(9);
	utoa(global_color_value[3][0], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[3][1], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[3][2], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[3][3], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S4:");
	usart_transmit_char(9);
	utoa(global_color_value[4][0], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[4][1], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[4][2], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[4][3], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S5:");
	usart_transmit_char(9);
	utoa(global_color_value[5][0], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[5][1], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[5][2], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(9);
	utoa(global_color_value[5][3], string, 10);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
}

// Transmit the percent change array over USART
void color_transmit_change(void)
{
	char string[12];
	
	usart_transmit_string("S0:");
	usart_transmit_char(9);
	dtostrf(global_color_change[0][0], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[0][1], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[0][2], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[0][3], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S1:");
	usart_transmit_char(9);
	dtostrf(global_color_change[1][0], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[1][1], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[1][2], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[1][3], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S2:");
	usart_transmit_char(9);
	dtostrf(global_color_change[2][0], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[2][1], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[2][2], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[2][3], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S3:");
	usart_transmit_char(9);
	dtostrf(global_color_change[3][0], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[3][1], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[3][2], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[3][3], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S4:");
	usart_transmit_char(9);
	dtostrf(global_color_change[4][0], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[4][1], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[4][2], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[4][3], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
	usart_transmit_string("S5:");
	usart_transmit_char(9);
	dtostrf(global_color_change[5][0], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[5][1], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[5][2], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(9);
	dtostrf(global_color_change[5][3], 10, 5, string);
	usart_transmit_string(string);
	usart_transmit_char(NEWLINE);
}