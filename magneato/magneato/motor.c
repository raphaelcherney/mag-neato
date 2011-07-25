/*
 * motor.c
 *
 * Created: 6/28/2011 12:52:38 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "motor.h"
#include "accel.h"
#include "clock.h"

/* ---GLOBAL VARIABLES--- */
volatile signed int global_left_encoder = 0;
volatile signed int global_right_encoder = 0;
extern volatile char global_state;
extern float global_desired_angle;

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

void motor_set_power(char motor, char direction, unsigned int power)
{
	switch (motor)
	{
		case LEFT:
			switch (direction)
			{
				case REVERSE:
					PORTD.OUTCLR = 0b00000100;
					PORTD.OUTSET = 0b00000010;
					break;
				case FORWARD:
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
				case REVERSE:
					PORTD.OUTCLR = 0b00001000;
					PORTD.OUTSET = 0b00010000;
					break;
				case FORWARD:
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

void motor_drive(char direction, unsigned int left_motor_power, unsigned int right_motor_power)
{
	TCD0.CCA = left_motor_power;
	TCD1.CCB = right_motor_power;
	switch (direction)
	{
		case REVERSE:
			PORTD.OUTCLR = 0b00001100;
			PORTD.OUTSET = 0b00010010;
			break;
		case FORWARD:
			PORTD.OUTCLR = 0b00010010;
			PORTD.OUTSET = 0b00001100;
			break;
		case STOP:
			PORTD.OUTCLR = 0b00011110;
			break;
	}
}

void motor_turn_to_angle(float desired_angle)	// TODO: take input of max_error
{
	float actual_angle, error, control;
	float integral = 0;
	float P = 40000;
	float I = 20;
	
	motor_enable();
	do
	{
		actual_angle = accel_get_heading();				// get heading
		//error = calculate_error(desired, actual);	// calculate error (negative results should turn right)
		error = valid_angle(desired_angle - actual_angle);
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
	} while (fabs(error) > 0.01);
	motor_disable();
}

void motor_follow_heading(float desired_heading, unsigned int base_power)
{
	float actual, error, control;
	float integral = 0;
	float P = 10000;
	float I = 20;
	long left_control, right_control;
	
	//motor_turn_arc(STOP, 0, 0);
	motor_enable();
	while (global_state == FOLLOW_HEADING)
	{
		actual = accel_get_heading();						// get heading
		error = calculate_error(desired_heading, actual);	// calculate error (negative results should turn right)
		integral = integral + error;						// sum error over time
		control = error * P + integral * I;					// calculate adjustment to base power
		
		left_control = fmin((long) base_power - control, MAX);
		right_control = fmin((long) base_power + control, MAX);	
		
		if (left_control >= 0) motor_set_power(LEFT, FORWARD, fmin((unsigned int) left_control, MAX));
		else motor_set_power(LEFT, REVERSE, fmin((unsigned int) fabs(left_control), MAX));
		
		if (right_control >= 0) motor_set_power(RIGHT, FORWARD, fmin((unsigned int) right_control, MAX));
		else motor_set_power(RIGHT, REVERSE, fmin((unsigned int) fabs(right_control), MAX));
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

void motor_encoder_drive(char direction, unsigned int counts)
{
	motor_drive(direction, MAX, MAX);
	motor_enable();
	global_left_encoder = 0;
	global_right_encoder = 0;
	while (global_left_encoder < counts) nop();
	motor_disable();
}