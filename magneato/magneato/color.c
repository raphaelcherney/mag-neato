/*
 * color.c
 *
 * Created: 6/29/2011 11:07:17 AM
 *  Author: Raphael
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "color.h"

/* ---GLOBAL VARIABLES--- */
unsigned int global_color_value[6][4];
unsigned int global_color_calibrate[6][4];
float global_color_change[6][4];
char global_color_sensor_count;
char global_color_filter;
float white[4] = {0, 0, 0, 0};
float black[4] = {1, 0.9, 1, 1};
float red[4] = {0.35, 0.1, 0.5, 0.5};
float green[4] = {0.2, 0.4, 0.15, 0.17};
float blue[4] = {0.4, 0.9, 0.4, 0.18};

/* ---FUNCTION DEFINITIONS--- */
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

void color_init(void)
{
	color_set_frequency(ONE_HUNDRED_PERCENT);
	
	TCE1.CTRLA = 0b00000001;		// enable TCE1 with prescaler: clk/1 (will overflow every 2ms)
	TCE1.INTCTRLA = 0b00000001;		// configure timer overflow as LOW level interrupt
	
	TCE0.CTRLD = 0b10101000;		// frequency capture using event CH0
	TCE0.CTRLB |= 0b00010000;		// enable CCA
	TCE0.CTRLA = 0b00000001;		// enable TCE0 with prescaler: clk/1
	
	EVSYS.CH0MUX = 0b01110000;		// set PE0 as CH0 event trigger (assign starting state)
	global_color_sensor_count = 0;	// measuring sensor 0
	color_set_filter(CLEAR);
	global_color_filter = 0;
}

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
					//color_transmit_change();
					break;
			}
			break;
	}
}

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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
}

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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
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
	usart_transmit_char(12);
}