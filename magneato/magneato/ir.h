/*
 * ir.h
 *
 * Created: 7/5/2011 10:33:54 AM
 *  Author: Raphael
 */ 


#ifndef IR_H
#define IR_H

/* ---FUNCTION PROTOTYPES--- */
void ir_enable(void);
void ir_disable(void);
void ir_set_threshold(char level);
void ir_change_threshold(signed char amount);

#endif /* IR_H */