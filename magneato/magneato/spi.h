/*
 * spi.h
 *
 * Created: 8/9/2011 2:35:45 PM
 *  Author: Raphael
 */ 


#ifndef SPI_H
#define SPI_H

/* ---FUNCTION PROTOTYPES--- */
void spi_enable(void);
void spi_write(unsigned char data);
char spi_read(void);

#endif /* SPI_H */