/*
 * led.h
 *
 * Created: 6/28/2011 2:49:31 PM
 *  Author: Raphael Cherney
 */ 


#ifndef LED_H
#define LED_H

/* ---FUNCTION PROTOTYPES--- */
void led_set(char led);
void led_toggle(char led);
void led_clear(char led);
void led_clear_all(void);

#endif /* LED_H */