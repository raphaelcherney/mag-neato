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
extern float white[4];
extern float black[4];
extern float red[4];
extern float green[4];
extern float blue[4];

/* ---MAIN FUNCTION--- */
int main(void)
{
	char string[12];
	unsigned int x, y, z;
	unsigned int c0, c1, c2, c3, c4, c5;
	float P, I, D;
	float actual, desired, error, integral, derivative, last_error, control;
	coordinate accel;
	int temp;
	char i, j;
	char white_flag, black_flag, red_flag, green_flag, blue_flag;
	float white_dif, black_dif, red_dif, green_dif, blue_dif;
	float change[6][4];
	
	led_set(UNDER);
	
	init();
	
	//ir_enable();
	//set_ir_threshold(52);
	
	led_clear_all();
	
	global_state = STOP;
	desired = M_PI_2;	// set desired heading
	P = 30000;
	I = 50;
	D = 0;
	
	//enable_motors();
	//global_state = MID_LEFT;
	//update_motor_state(global_state);
	usart_init();
	led_set(UNDER);
	//led_set(GREEN);
	
	while(1)
    {
		for (i=0; i<6; i++)
		{
			for (j=0; j<4; j++)
			{
				white_dif += fabs(global_color_change[i][j] - white[j]);
				black_dif += fabs(global_color_change[i][j] - black[j]);
				red_dif += fabs(global_color_change[i][j] - red[j]);
				green_dif += fabs(global_color_change[i][j] - green[j]);
				blue_dif += fabs(global_color_change[i][j] - blue[j]);
				// smallest difference should be color
			}
			if (black_dif<white_dif && black_dif<red_dif && black_dif<green_dif && black_dif<blue_dif)
			{
				black_flag = 1;
			}
			else if (red_dif<white_dif && red_dif<black_dif && red_dif<green_dif && red_dif<blue_dif)
			{
				red_flag = 1;
			}
			else if (green_dif<white_dif && green_dif<black_dif && green_dif<red_dif && green_dif<blue_dif)
			{
				green_flag = 1;
			}
			else if (blue_dif<white_dif && blue_dif<black_dif && blue_dif<red_dif && blue_dif<green_dif)
			{
				blue_flag = 1;
			}
			else
			{
				white_flag = 1;
			}
			
			dtostrf(black_dif, 10, 5, string);
			usart_transmit_string(string);
			usart_transmit_char(12);
			
			white_dif = 0;
			black_dif = 0;
			red_dif = 0;
			green_dif = 0;
			blue_dif = 0;
		}
		
		if (blue_flag)
		{
			led_set(BLUE);
		}
		else
		{
			led_clear(BLUE);
		}
		
		if (red_flag)
		{
			led_set(RED);
		}
		else
		{
			led_clear(RED);
		}
		
		if (green_flag)
		{
			led_set(GREEN);
		}
		else
		{
			led_clear(GREEN);
		}
		
		if (black_flag)
		{
			led_set(YELLOW);
		}
		else
		{
			led_clear(YELLOW);
		}
		
		white_flag = 0;
		black_flag = 0;
		red_flag = 0;
		green_flag = 0;
		blue_flag = 0;
		
		/*
		for (i=0; i<6; i++)
		{
			if ((global_color_value[i][0]+global_color_value[i][1]+global_color_value[i][2]+global_color_value[i][3])
					> (global_color_calibrate[i][0]+global_color_calibrate[i][1]+global_color_calibrate[i][2]+global_color_calibrate[i][3]+70)) // there is a line
			{
				flag = 1;
			}
		}
		
		if (flag == 1)
		{
			led_set(BLUE);
		}
		else
		{
			led_clear(BLUE);
		}
		flag = 0;	
		*/				
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