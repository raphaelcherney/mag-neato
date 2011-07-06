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

/* ---FUNCTION PROTOTYPES--- */
void init(void);
void ir_enable(void);
void ir_disable(void);
void ir_set_threshold(char level);
void ir_change_threshold(signed char amount);
void update_motor_state(char state);
void color_change(void);
float percent_change(float number, float reference);

/* ---GLOBAL VARIABLES--- */
char global_state = STOP;
extern unsigned int global_color_value[6][4];
extern unsigned int global_color_calibrate[6][4];
extern float global_color_change[6][4];
extern char global_color_new;
extern signed int global_left_encoder;
extern signed int global_right_encoder;

/* ---MAIN FUNCTION--- */
int main(void)
{
	unsigned char sensor, filter;
	char colors[6];
	
	init();
	led_set(UNDER);
	
	//enable_motors();
	//global_state = MID_LEFT;
	//update_motor_state(global_state);
	//usart_init();
	
	//led_set(GREEN);
	
	while(1)
    {
		colors[0] = WHITE;
		colors[1] = WHITE;
		colors[2] = WHITE;
		colors[3] = WHITE;
		colors[4] = WHITE;
		colors[5] = WHITE;
		
		for (sensor=0; sensor<6; sensor++)
		{
			for (filter=0; filter<4; filter++)
			{
				if (global_color_change[sensor][filter] > 0.3)	// if percent change from calibration on any sensor is > 30%
				{
					if (global_color_change[sensor][0] > 0.5 &&
						global_color_change[sensor][1] > 0.5 &&
						global_color_change[sensor][2] > 0.5 &&
						global_color_change[sensor][3] > 0.5)
					{
						colors[sensor] = BLACK;
					}
					else if (global_color_change[sensor][1] < global_color_change[sensor][2] && 
							 global_color_change[sensor][1] < global_color_change[sensor][3])
					{
						colors[sensor] = RED;
					}
					else if (global_color_change[sensor][2] < global_color_change[sensor][1] && 
							 global_color_change[sensor][2] < global_color_change[sensor][3])
					{
						colors[sensor] = GREEN;
					}
					else
					{
						colors[sensor] = BLUE;
					}
				}
			}
		}
		
		led_clear(YELLOW);
		led_clear(RED);
		led_clear(GREEN);
		led_clear(BLUE);
		for (sensor=0; sensor<6; sensor++)
		{
			if (colors[sensor] == BLACK) led_set(YELLOW);
			if (colors[sensor] == RED) led_set(RED);
			if (colors[sensor] == GREEN) led_set(GREEN);
			if (colors[sensor] == BLUE) led_set(BLUE);
		}			
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
	PORTC.DIR = 0b00000000;		// all PORTC as inputs
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
	motor_enable();
	
	/* ---ENCODERS--- */
	motor_encoder_enable();
	motor_encoder_set_threshold(2);
	
	/* ---ACCELEROMETER--- */
	accel_init();
	
	/* ---COLOR SENSORS--- */
	color_init();
	
	/* ---REFLECTIVE IR SENSORS--- */
	ACA.AC0MUXCTRL = 0b00101111;	// use PA5 and scaled VCC for ACA0
	ACA.AC1MUXCTRL = 0b00110111;	// use PA6 and scaled VCC for ACA1
	ACA.CTRLB = 63;					// set initial scaled VCC level
	//ACA.AC0CTRL = 0b11101101;		// trigger MID level interrupt on rising edge using high-speed mode and large hysteresis
	//ACA.AC1CTRL = 0b11101101;		// enable ACA1 with same settings
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