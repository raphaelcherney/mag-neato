/*
 * color.h
 *
 * Created: 6/29/2011 11:09:40 AM
 *  Author: Raphael
 */ 


#ifndef COLOR_H
#define COLOR_H

/* ---FUNCTION PROTOTYPES--- */
void color_set_filter(char color);
void color_set_frequency(char scale);
void color_init(void);
void color_update(void);
void color_calibrate(void);
void color_compare(void);
void color_change(void);
void color_check_for_red(void);
void color_transmit_value(void);
void color_transmit_change(void);

#endif /* COLOR_H */