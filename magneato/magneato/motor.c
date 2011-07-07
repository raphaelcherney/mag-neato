/*
 * motor.c
 *
 * Created: 6/28/2011 12:52:38 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"

/* ---GLOBAL VARIABLES--- */
signed int global_left_encoder = 0;
signed int global_right_encoder = 0;

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
	float P, I, D, actual, error, integral, derivative, last_error, control;
	
	P = 30000;
	I = 50;
	D = 0;
	
	while (error > 0.05)	// while error is greater than around 3 degrees
	{
		actual = accel_get_heading();				// get heading
		error = calculate_error(desired, actual);	// calculate error (negative results should turn right)
		integral = integral + error;				// sum error over time
		derivative = error - last_error;			// find the rate of change in error
		last_error = error;							// update trailing error value
		control = fmax(fmin(error * P + integral * I + derivative * D, MAX), -MAX);
		if (control >= 0)	// go left
		{
			motor_set_power(LEFT, REVERSE, (unsigned int) control);
			motor_set_power(RIGHT, FORWARD, (unsigned int) control);
		}
		else				// go right
		{
			motor_set_power(LEFT, FORWARD, (unsigned int) fabs(control));
			motor_set_power(RIGHT, REVERSE, (unsigned int) fabs(control));
		}
	}
	
		
		/*
		
		actual = heading(get_accel());		// get heading
		error = desired - actual;			// calculate error (negative results mean you should turn right)
		integral = integral + error;		// sum up error over time
		derivative = error - last_error;	// find the rate of change in error
		last_error = error;					// update the trailing error value
		control = fmax(fmin(error * P + integral * I + derivative * D, MAX), -MAX);
		*/
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