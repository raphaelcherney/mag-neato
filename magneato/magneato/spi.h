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
void spi_transmit(unsigned char data);
char spi_read(void);
void spi_write_register(char register_address, char data);
void spi_read_register(char register_address, int num_bytes, char *data);

#endif /* SPI_H */