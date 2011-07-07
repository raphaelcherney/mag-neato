/*
 * motor.c
 *
 * Created: 6/28/2011 12:52:38 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>
#include <avr/interrupt.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "accel.h"

/* ---GLOBAL VARIABLES--- */
signed int global_left_encoder = 0;
signed int global_right_encoder = 0;
extern char global_state;

/* ---FUNCTION DEFINITIONS--- */
void motor_enable(void)
{
	TCD0.CTRLA = 0b00000001;	// prescaler: clk/1
	TCD0.CTRLB = 0b00010011;	// set PD0 as single slope PWM output
	TCD1.CTRLA = 0b00000001;	// prescaler: clk/1
	TCD1.CTRLB = 0b00100011;	// set PD5 as single slope PWM output
	// TODO: may want to move this into a motor_config function instead?
	PORTD.OUTSET = 0b01000000;	// enable motor controller
}

void motor_disable(void)
{
	PORTD.OUTCLR = 0b01000000;	// disable motor controller
}

void motor_set_power(char motor, char direction, int power)
{
	switch (motor)
	{
		case LEFT:
			switch (direction)
			{
				case FORWARD:
					PORTD.OUTCLR = 0b00000100;
					PORTD.OUTSET = 0b00000010;
					break;
				case REVERSE:
					PORTD.OUTCLR = 0b00000010;
					PORTD.OUTSET = 0b00000100;
					break;
				case STOP:
					PORTD.OUTCLR = 0b00000110;
					break;
			}
			TCD0.CCA = power;
			break;
		case RIGHT:
			switch (direction)
			{
				case FORWARD:
					PORTD.OUTCLR = 0b00001000;
					PORTD.OUTSET = 0b00010000;
					break;
				case REVERSE:
					PORTD.OUTCLR = 0b00010000;
					PORTD.OUTSET = 0b00001000;
					break;
				case STOP:
					PORTD.OUTCLR = 0b00011000;
					break;
			}
			TCD1.CCB = power;
			break;
	}
}

void motor_turn_arc(char direction, int left_motor_power, int right_motor_power)
{
	switch (direction)
	{
		case FORWARD:
			PORTD.OUTCLR = 0b00001100;
			PORTD.OUTSET = 0b00010010;
			TCD0.CCA = left_motor_power;
			TCD1.CCB = right_motor_power;
			break;
		case REVERSE:
			PORTD.OUTCLR = 0b00010010;
			PORTD.OUTSET = 0b00001100;
			TCD0.CCA = left_motor_power;
			TCD1.CCB = right_motor_power;
			break;
	}
}

void motor_turn_to_angle(float desired)
{
	float actual, error, integral, control;
	float P = 40000;
	float I = 10;
	
	error = 1;
	motor_enable();
	while (fabs(error) > 0.001)
	{
		actual = accel_get_heading();				// get heading
		error = calculate_error(desired, actual);	// calculate error (negative results should turn right)
		integral = integral + error;				// sum error over time
		control = fmax(fmin(error * P + integral * I, MAX), -MAX);
		if (control >= 0)	// go left
		{
			motor_set_power(LEFT, REVERSE, (int) control);
			motor_set_power(RIGHT, FORWARD, (int) control);
		}
		else				// go right
		{
			motor_set_power(LEFT, FORWARD, (int) fabs(control));
			motor_set_power(RIGHT, REVERSE, (int) fabs(control));
		}
	}
	motor_disable();
}

void motor_follow_heading(float desired_heading, char direction, unsigned int base_power)
{
	float actual, error, integral, control;
	float P = 40000;
	float I = 10;
	long base, left_control, right_control;
	
	if (direction == FORWARD) base = (long) base_power;
	else if (direction == REVERSE) base = -(long) base_power;
	
	motor_enable();
	while (global_state == FOLLOW_HEADING)
	{
		actual = accel_get_heading();						// get heading
		error = calculate_error(desired_heading, actual);	// calculate error (negative results should turn right)
		integral = integral + error;						// sum error over time
		control = error * P + integral * I;					// calculate adjustment to base power
		
		left_control = base + (long) control;
		right_control = base + (long) control;
		
		if (left_control >= 0) motor_set_power(LEFT, FORWARD, fmin((int) left_control, MAX));
		else motor_set_power(LEFT, REVERSE, fmin(fabs(left_control) ,MAX));
		
		if (right_control >= 0) motor_set_power(RIGHT, FORWARD, fmin((int) right_control, MAX));
		else motor_set_power(RIGHT, REVERSE, fmin(fabs(right_control) ,MAX));
	}
	motor_disable();
}

void motor_encoder_enable(void)
{
	PORTD.OUTSET = 0b10000000;		// turn on encoder emitter
	ACB.AC0MUXCTRL = 0b00000111;	// use PB0 and scaled VCC for ACB0
	ACB.AC1MUXCTRL = 0b00001111;	// use PB1 and scaled VCC for ACB1
	ACA.CTRLB = 63;					// set initial scaled VCC level
	ACB.AC0CTRL = 0b00101101;		// trigger MID level interrupt on both edges using high-speed mode and large hysteresis
	ACB.AC1CTRL = 0b00101101;		// enable ACB1 with same settings
	TCC0.CTRLA = 0b00000101;		// enable TCC0 with clk/64 (2 us period)
	TCC1.CTRLA = 0b00000101;		// enable TCC1 with clk/64 (2 us period)
}

void motor_encoder_disable(void)
{
	
	PORTD.OUTCLR = 0b10000000;		// turn off encoder emitter
	ACB.AC0CTRL &= ~(0b0000001);	// disable ACB0
	ACB.AC1CTRL &= ~(0b0000001);	// disable ACB1
	TCC0.CTRLA = 0b00000000;		// disable TCC0
	TCC1.CTRLA = 0b00000000;		// disable TCC0
}

void motor_encoder_set_threshold(char level)
{
	ACB.CTRLB = level;
}