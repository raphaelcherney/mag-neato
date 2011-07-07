/*
 * interrupt.c
 *
 * Created: 6/29/2011 11:39:45 AM
 *  Author: Raphael Cherney
 */ 

// TODO: make multibyte R/W atomic

/* ---AVR HEADER FILES--- */
#include <avr/io.h>
#include <avr/interrupt.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"

/* ---GLOBAL VARIABLES--- */
extern char global_state;
extern signed int global_left_encoder;
extern signed int global_right_encoder;
extern unsigned int global_color_value[6][4];
extern unsigned int global_color_calibrate[6][4];
extern char global_color_sensor_count;
extern char global_color_filter;

/* ---INTERRUPT SERVICE ROUTINES--- */
ISR(ACB_AC0_vect)		// LEFT WHEEL ENCODER
{
	unsigned int minimum_time;
	
	//minimum_time =	34768 - (TCD0.CCA >> 1);		// define minimum pulse width off of motor power
	minimum_time = 2000;
	
	if ((TCC0.CNT > minimum_time) || (TCC0.INTFLAGS & 0x01))	// if enough time has passed that it will not double count a transition...
	{
		global_left_encoder++;			// add encoder count
		led_toggle(RED);
		// TODO: this should be direction dependent!
		TCC0.INTFLAGS = 0b00000001;		// clear flag
	}
	TCC0.CNT = 0;						// reset counter
}

ISR(ACB_AC1_vect)		// RIGHT WHEEL ENCODER
{
	unsigned int minimum_time;
	
	//minimum_time =	34768 - (TCD1.CCB >> 1);	// define minimum pulse width off of motor power
	minimum_time = 2000;
	
	if ((TCC1.CNT > minimum_time) || (TCC1.INTFLAGS & 0x01))	// if enough time has passed that it will not double count a transition...
	{
		global_right_encoder++;			// add encoder count
		led_toggle(YELLOW);
		// TODO: this should be direction dependent!
		TCC1.INTFLAGS = 0b00000001;		// clear flag
	}
	TCC1.CNT = 0;						// reset counter
}

ISR(PORTA_INT0_vect)	// LEFT BUMPER
{
	motor_disable();
	led_set(RED);
}

ISR(PORTF_INT0_vect)	// RIGHT BUMPER
{
	motor_disable();
	led_set(RED);
}

ISR(PORTB_INT0_vect)	// USER PUSHBUTTON SW0
{
	motor_disable();
}

ISR(PORTB_INT1_vect)	// USER PUSHBUTTON SW1
{
	color_calibrate();
	global_left_encoder = 0;
	global_right_encoder = 0;
	motor_enable();
}

/*
ISR(ACA_AC0_vect)		// LEFT REFLECTIVE IR SENSOR
{
	if (global_state == LEFT || global_state == STRAIGHT)
	{
		toggle_led(GREEN);
		//set_motor_power(RIGHT, FORWARD, MAX);
		//set_motor_power(LEFT, STOP, 0);
		delay(255);
		global_state = RIGHT;
	}	
}

ISR(ACA_AC1_vect)		// RIGHT REFLECTIVE IR SENSOR
{
	if (global_state == RIGHT || global_state == STRAIGHT)
	{
		toggle_led(GREEN);
		//set_motor_power(LEFT, FORWARD, MAX);
		//set_motor_power(RIGHT, STOP, 0);
		delay(255);
		global_state = LEFT;
	}
}
*/

ISR(TCE1_OVF_vect)		// TCE1 OVERFLOW
{
	color_update();
}

/* ---FUNCTION DEFINITIONS--- */
void interrupt_enable(char level)
{
	sei();						// enable global interrupts
	switch (level)
	{
		case LOW:
			PMIC.CTRL |= 0b00000001;
			break;
		case MID:
			PMIC.CTRL |= 0b00000010;
			break;
		case HIGH:
			PMIC.CTRL |= 0b00000100;
			break;
		case ALL:
			PMIC.CTRL |= 0b00000111;
			break;
	}
}

void interrupt_disable(char level)
{
	switch (level)
	{
		case LOW:
			PMIC.CTRL &= ~(0b00000001);
			break;
		case MID:
			PMIC.CTRL &= ~(0b00000010);
			break;
		case HIGH:
			PMIC.CTRL &= ~(0b00000100);
			break;
		case ALL:
			PMIC.CTRL &= ~(0b00000111);
			cli();
			break;
	}
}