/*
 * usart.h
 *
 * Created: 6/30/2011 4:40:01 PM
 *  Author: Raphael
 */ 


#ifndef USART_H
#define USART_H

/* ---FUNCTION PROTOTYPES--- */
void usart_init(void);
void usart_transmit_char(unsigned char data);
void usart_transmit_int(int data);
void usart_transmit_unsigned_int(unsigned int data);
void usart_transmit_float(float data);
void usart_transmit_string(const char *str);

#endif /* USART_H */