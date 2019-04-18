#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "sapi.h"

#define N_BUTTON 4

typedef enum {
	UP,
	DOWN,
} Buttons_State;

typedef struct {
	Buttons_State state;
	gpioMap_t pin;
	uint32_t begin_time;
	uint32_t end_time;
} Buttons_t;

#endif
