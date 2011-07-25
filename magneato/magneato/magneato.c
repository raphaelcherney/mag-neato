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

/* ---FUNCTION PROTOTYPES--- */
void init(void);
void ir_enable(void);
void ir_disable(void);
void ir_set_threshold(char level);
void ir_change_threshold(signed char amount);
void update_motor_state(char state);

/* ---GLOBAL VARIABLES--- */
volatile char global_state = STOP;
volatile char global_program = STOP;
extern unsigned int global_color_value[6][4];
extern unsigned int global_color_calibrate[6][4];
extern float global_color_change[6][4];
extern volatile signed int global_left_encoder;
extern volatile signed int global_right_encoder;
volatile float global_desired_angle = M_PI_4;

/* ---MAIN FUNCTION--- */
int main(void)
{
	volatile unsigned char i;
	float heading;
	coordinate_3d accelerations;
	
	init();
	motor_turn_to_angle(M_PI_4);
	
	while(1)
    {
		accelerations = accel_get();
		usart_transmit_string("x=");
		usart_transmit_int(accelerations.x);
		usart_transmit_char(TAB);
		usart_transmit_string("y=");
		usart_transmit_int(accelerations.y);
		usart_transmit_char(TAB);
		usart_transmit_string("z=");
		usart_transmit_int(accelerations.z);
		heading = accel_calculate_heading(accelerations);
		usart_transmit_char(TAB);
		usart_transmit_string("heading=");
		usart_transmit_float(heading);
		usart_transmit_char(NEWLINE);
		if (heading > 0)
		{
			led_set(GREEN);
		}
		else
		{
			led_clear(GREEN);
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
	//motor_enable();
	
	/* ---ENCODERS--- */
	motor_encoder_enable();
	motor_encoder_set_threshold(6);
	
	/* ---ACCELEROMETER--- */
	accel_init();
	
	/* ---COLOR SENSORS--- */
	color_init();
	
	/* ---USART--- */
	usart_init();
	
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