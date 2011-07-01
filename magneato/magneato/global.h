/*
 * global.h
 *
 * Created: 6/28/2011 1:02:45 PM
 *  Author: Raphael
 */ 


#ifndef GLOBAL_H_
#define GLOBAL_H_

/* ---CUSTOM STRUCTURES--- */
typedef struct {
	int x, y, z;
} coordinate;

/* ---DEFINE CONSTANTS--- */
#define STOP 0		// motor directions
#define FORWARD 1
#define REVERSE 2
#define LEFT 0		// identifier
#define RIGHT 1
#define STRAIGHT 2
#define MAX 65535	// maximum motor speed
#define CLEAR 0		// color identifiers
#define RED 1		
#define GREEN 2
#define BLUE 3
#define YELLOW 4
#define UNDER 5
#define X 0			// accelerometer directions
#define Y 1
#define Z 2
#define OFF 0					// color sensor settings
#define TWO_PERCENT 1
#define TWENTY_PERCENT 2
#define ONE_HUNDRED_PERCENT 3
#define HARD_LEFT 10		// states
#define MID_LEFT 20
#define SLIGHT_LEFT 30
#define SLIGHT_RIGHT 40
#define MID_RIGHT 50
#define HARD_RIGHT 60
#define LOW 100
#define MID 101
#define HIGH 102
#define ALL	103

#endif /* GLOBAL_H_ */