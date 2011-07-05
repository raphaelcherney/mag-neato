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
void color_change(void);
void color_transmit(void);

#endif /* COLOR_H */