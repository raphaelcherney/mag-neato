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
#include <math.h>
#define F_CPU 32000000UL		// 32 MHz
#include <util/delay.h>			// delay functions

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "motor.h"
#include "led.h"
#include "color.h"

/* ---GLOBAL VARIABLES--- */
extern volatile char global_state;
extern volatile char global_program;
extern signed int global_left_encoder;
extern signed int global_right_encoder;
extern volatile unsigned int global_color_value[6][4];
extern volatile unsigned int global_color_calibrate[6][4];
extern volatile char global_color_sensor_count;
extern volatile char global_color_filter;
extern volatile float global_desired_angle;

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
	switch (global_program)
    {
        case LINE_FOLLOW:
            motor_disable();
            led_set(RED);
            break;
        case BOUNCE:
            global_state = REVERSE_LEFT;
			break;
    }
}

ISR(PORTF_INT0_vect)	// RIGHT BUMPER
{
	switch (global_program)
    {
        case LINE_FOLLOW:
            motor_disable();
            led_set(RED);
            break;
        case BOUNCE:
            global_state = REVERSE_RIGHT;
			break;
    }
}

ISR(PORTB_INT0_vect)	// USER PUSHBUTTON SW0
{
	global_program = BOUNCE;
	global_state = TURN;
	led_clear(ALL);
	led_set(UNDER);
	_delay_ms(500);
	color_calibrate();
	_delay_ms(50);
}

ISR(PORTB_INT1_vect)	// USER PUSHBUTTON SW1
{
	global_program = LINE_FOLLOW;
	global_state = START;
	led_set(UNDER);
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

ISR(TCE1_CCA_vect)		// TCE1 CCA INTERRUPT (COLOR)
{
	color_update();
	TCE1.CCA = 0;		// reset counter
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