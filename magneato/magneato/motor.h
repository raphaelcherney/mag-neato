/*
 * motor.h
 *
 * Created: 6/28/2011 12:52:53 PM
 *  Author: Raphael Cherney
 */ 

#ifndef MOTOR_H
#define MOTOR_H

/* ---FUNCTION PROTOTYPES--- */
void motor_enable(void);
void motor_disable(void);
void motor_set_power(char motor, char direction, int power);
void motor_set_speed(char motor, char direction, int speed);	// NOT IMPLEMENTED
void motor_turn_arc(char direction, int left_motor_power, int right_motor_power);
void motor_turn_to_angle(float heading);	// NOT IMPLEMENTED
void motor_encoder_enable(void);
void motor_encoder_disable(void);
void motor_encoder_set_threshold(char level);

#endif /* MOTOR_H */