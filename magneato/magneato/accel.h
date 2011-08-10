/*
 * accel.h
 *
 * Created: 6/29/2011 1:41:06 PM
 *  Author: Raphael Cherney
 */ 


#ifndef ACCEL_H
#define ACCEL_H

/* ---FUNCTION PROTOTYPES--- */
void accel_init(void);
coordinate_3d accel_get(void);
float accel_calculate_heading(coordinate_3d accel);
float accel_get_heading(void);
coordinate_3d accel_get_spi(void);

#endif /* ACCEL_H */