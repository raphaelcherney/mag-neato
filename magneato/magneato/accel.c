/*
 * accel.c
 *
 * Created: 6/29/2011 1:38:54 PM
 *  Author: Raphael Cherney
 */ 

/* ---AVR HEADER FILES--- */
#include <avr/io.h>
#include <math.h>

/* ---LOCAL HEADER FILES--- */
#include "global.h"
#include "accel.h"
#include "spi.h"

/* ---FUNCTION DEFINITIONS--- */
void accel_init(void)
{
	ADCA.CTRLA = 0b00000001;			// enable ADCA
	ADCA.PRESCALER = 0b00000100;		// prescaler: clk/64
	ADCA.REFCTRL = 0b00010000;			// use Vcc/1.6 as reference voltage
	ADCA.CH0.CTRL = 0b00000001;			// use single-ended input with 1x gain
	ADCA.CH1.CTRL = 0b00000001;			// use single-ended input with 1x gain
	ADCA.CH2.CTRL = 0b00000001;			// use single-ended input with 1x gain
	ADCA.CH0.MUXCTRL = 0b00010000;		// use PA2 pin
	ADCA.CH1.MUXCTRL = 0b00011000;		// use PA3 pin
	ADCA.CH2.MUXCTRL = 0b00100000;		// use PA4 pin
	// TODO: wait for ADC to start up
}

coordinate_3d accel_get(void)
{	
	coordinate_3d accel;
	
	ADCA.CTRLA |= 0b00011100;				// start CH0-CH2 conversions
	while(!(ADCA.INTFLAGS & 0b00000001));	// wait for CH0 interrupt flag
	accel.x = ADCA.CH0.RES;					// read ADC result
	while(!(ADCA.INTFLAGS & 0b00000010));	// wait for CH1 interrupt flag
	accel.y = ADCA.CH1.RES;
	while(!(ADCA.INTFLAGS & 0b00000100));	// wait for CH2 interrupt flag
	accel.z = ADCA.CH2.RES;

	return(accel);
}

float accel_calculate_heading(coordinate_3d accel)
{
	float heading;
	float Vzero = 1.6;		// should be 1.6 V
	float Vdd = 3.3;
	float Vref = Vdd / 1.6;
	float deltaV = Vref * 0.05;;
	float Vx, Vy, Vz;
	float x, y;
	
	Vx = ((float)accel.x * Vref) / 4096 - deltaV;
	Vy = ((float)accel.y * Vref) / 4096 - deltaV;
	Vz = ((float)accel.z * Vref) / 4096 - deltaV;
	
	Vx = Vx - Vzero;
	Vy = Vy - Vzero;
	Vz = Vz - Vzero;
	
	x = -Vy;	// change sign due to board layout
	y = -Vx;
	
	heading = atan2(y, x);
	
	return(heading);
}

float accel_get_heading(void)		// TODO: this function can be reprogrammed to run faster
{
	coordinate_3d accel;
	float heading;
	
	accel = accel_get();
	heading = accel_calculate_heading(accel);
	
	return(heading);
}

coordinate_3d accel_get_spi(void)
{
	char POWER_CTL = 0x2D;
	char DATA_FORMAT = 0x31;
	char DATAX0 = 0x32;			//X-Axis Data 0
	char DATAX1 = 0x33;			//X-Axis Data 1
	char DATAY0 = 0x34;			//Y-Axis Data 0
	char DATAY1 = 0x35;			//Y-Axis Data 1
	char DATAZ0 = 0x36;			//Z-Axis Data 0
	char DATAZ1 = 0x37;			//Z-Axis Data 1
	char values[10];
	int x, y, z;
	coordinate_3d accel;
	
	spi_write_register(0x31, 0x01);
	spi_write_register(0x2D, 0x08);	// put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register

	spi_read_register(DATAX0, 6, values);
	accel.x = ((int)values[1]<<8)|(int)values[0];
	accel.y = ((int)values[3]<<8)|(int)values[2];
	accel.z = ((int)values[5]<<8)|(int)values[4];
	
	return accel;
}