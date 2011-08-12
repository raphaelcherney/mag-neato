/*
 * magneato.c
 *
 * Created: 6/15/2011 3:46:56 PM
 *  Author: Raphael Cherney
 */
	
/* ---AVR HEADER FILES--- */
#include <avr/io.h>				// device specific IO definitions
#include <avr/interrupt.h>		// interrupt definitions and macros
#include <stdlib.h>				// general utilities
#include <math.h>				// math functions
#define F_CPU 32000000UL		// 32 MHz
#include <util/delay.h>			// delay functions

/*---LOCAL HEADER FILES--- */
#include "global.h"
#include "interrupt.h"
#include "clock.h"
#include "accel.h"
#include "motor.h"
#include "led.h"
#include "color.h"
#include "usart.h"
#include "ir.h"
#include "spi.h"

/* ---FUNCTION PROTOTYPES--- */
void init(void);
void update_motor_state(char state);

/* ---GLOBAL VARIABLES--- */
volatile char global_state = STOP;
char global_program = STOP;
extern unsigned int global_color_value[6][4];
extern unsigned int global_color_calibrate[6][4];
extern float global_color_change[6][4];
extern char global_color_new;
extern volatile signed int global_left_encoder;
extern volatile signed int global_right_encoder;
float volatile global_desired_angle = M_PI_4;

/* ---MAIN FUNCTION--- */
int main(void)
{
	unsigned char i;
	float threshold = 0.25;
	coordinate_3d outputs;
	
	init();
	led_clear(ALL);
	
	spi_enable();
	
	while(1)
	{
		outputs = accel_get_spi();
	}
	
	//led_set(UNDER);
	//usart_init();
	
	//motor_encoder_drive(FORWARD, 40);
	
	while(1)
    {
		if (global_program == LINE_FOLLOW)
		{
			/*
			if (global_state == START)
			{
				_delay_ms(500);
				color_calibrate();
				_delay_ms(50);
				motor_enable();
				motor_drive(FORWARD, MAX, MAX);
				global_state = STRAIGHT;
			}
			for (i=0; i<4; i++)
			{
				if (global_color_change[0][i]>threshold)
				{
					update_motor_state(HARD_LEFT);
				}
				else if (global_color_change[1][i]>threshold)
				{
					update_motor_state(MID_LEFT);
				}
				else if (global_color_change[2][i]>threshold)
				{
					update_motor_state(SLIGHT_LEFT);
				}
				else if (global_color_change[3][i]>threshold)
				{
					update_motor_state(SLIGHT_RIGHT);
				}
				else if (global_color_change[4][i]>threshold)
				{
					update_motor_state(MID_RIGHT);
				}
				else if (global_color_change[5][i]>threshold)
				{
					update_motor_state(HARD_RIGHT);
				}
			}
			*/
			motor_drive(REVERSE, MAX, MAX);
			//motor_set_power(LEFT, FORWARD, MAX);
			//motor_set_power(RIGHT, FORWARD, MAX);
		}
		else if (global_program == BOUNCE)
		{
			if (global_state == REVERSE_LEFT || global_state == REVERSE_RIGHT)
			{
				//motor_encoder_drive(REVERSE, 20);
				motor_drive(REVERSE, MAX, MAX);
				motor_enable();
				_delay_ms(400);
				switch (global_state)
				{
					case REVERSE_LEFT:
						global_desired_angle = valid_angle(global_desired_angle - M_PI_2);
						break;
					case REVERSE_RIGHT:
						global_desired_angle = valid_angle(global_desired_angle + M_PI_2);
						break;
				}
				global_state = TURN;
			}
			else if (global_state == TURN)
			{
				motor_turn_to_angle(global_desired_angle);
				global_state = FOLLOW_HEADING;
			}
			else if (global_state == FOLLOW_HEADING)
			{
				motor_follow_heading(global_desired_angle, MAX);
			}
			else if (global_state == STRAIGHT)
			{
				motor_encoder_drive(FORWARD, 40);
				global_state = STOP;
			}
		}
		i++;
    }
}

/* ---FUNCTION DEFINITIONS--- */
void init(void)
{
	// TODO: add other options
	
	/* ---CLOCK INIT--- */
	clock_set_32mhz_crystal();
	
	/* ---GPIO INIT--- */
	PORTA.DIR = 0b00000001;		// PA0 as input, rest as outputs
	PORTB.DIR = 0b00000000;		// all PORTB as inputs
	PORTC.DIR = 0b10110010;		// setup PORTC for SPI
	PORTC.OUTSET = 0b00000010;	// set CS high
	PORTD.DIR = 0b11111111;		// all PORTD as outputs
	PORTE.DIR = 0b11111000;		// PE0-PE2 as inputs, rest as outputs
	PORTF.DIR = 0b01111000;		// PF3-PF6 as outputs, rest as inputs
	
	/* ---INTERRUPTS--- */
	interrupt_enable(ALL);
	
	/* ---BUMP SENSORS--- */
	PORTA.PIN1CTRL = 0b00000001;	// sense rising edge on PA1
	PORTA.INT0MASK = 0b00000010;	// PA1 as interrupt pin
	PORTA.INTCTRL = 0b00000011;		// enable INT0 as HIGH level interrupt
	PORTF.PIN7CTRL = 0b00000001;	// sense rising edge on PF7
	PORTF.INT0MASK = 0b10000000;	// PF7 as interrupt pin
	PORTF.INTCTRL = 0b00000011;		// enable INT0 as HIGH level interrupt
	// TODO: make into function so that you can turn it off
	
	/* ---USER PUSHBUTTONS--- */
	PORTB.PIN2CTRL = 0b00000010;	// sense falling edge on PB2
	PORTB.PIN3CTRL = 0b00000010;	// sense falling edge on PB3
	PORTB.INT0MASK = 0b00000100;	// PB2 as interrupt pin
	PORTB.INT1MASK = 0b00001000;	// PB3 as interrupt pin
	PORTB.INTCTRL = 0b00001111;		// enable INT0 as HIGH level interrupt
	
	/* ---MOTORS--- */
	motor_set_power(LEFT, STOP, 0);
	motor_set_power(RIGHT, STOP, 0);
	//motor_enable();
	
	/* ---ENCODERS--- */
	motor_encoder_enable();
	motor_encoder_set_threshold(6);
	
	/* ---ACCELEROMETER--- */
	accel_init();
	
	/* ---COLOR SENSORS--- */
	color_init();
	
	/* ---REFLECTIVE IR SENSORS--- */
	ir_init();
}

void update_motor_state(char state)
{
	switch (state)
	{
		case STOP:
			motor_set_power(LEFT, STOP, 0);
			motor_set_power(RIGHT, STOP, 0);
			break;
		case STRAIGHT:
			motor_set_power(LEFT, FORWARD, MAX);
			motor_set_power(RIGHT, FORWARD, MAX);
			break;
		case HARD_LEFT:
			motor_set_power(LEFT, STOP, 0);
			motor_set_power(RIGHT, FORWARD, MAX);
			break;
		case HARD_RIGHT:
			motor_set_power(LEFT, FORWARD, MAX);
			motor_set_power(RIGHT, STOP, 0);
			break;
		case MID_LEFT:
			motor_set_power(LEFT, FORWARD, MAX*.3);
			motor_set_power(RIGHT, FORWARD, MAX);
			break;
		case MID_RIGHT:
			motor_set_power(LEFT, FORWARD, MAX);
			motor_set_power(RIGHT, FORWARD, MAX*.3);
			break;
		case SLIGHT_LEFT:
			motor_set_power(LEFT, FORWARD, MAX*.75);
			motor_set_power(RIGHT, FORWARD, MAX);
			break;
		case SLIGHT_RIGHT:
			motor_set_power(LEFT, FORWARD, MAX);
			motor_set_power(RIGHT, FORWARD, MAX*.75);
			break;
	}
}