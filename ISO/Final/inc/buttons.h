#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "sapi.h"

#define N_BUTTON 2

#define BUTTON1 0
#define BUTTON2 1

#define ANTIREBOTE 20

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

void buttons_begin(void);
void tecla_presionada(gpioMap_t pin,Buttons_State estado);
void Tecla_Reiniciar_Inicio(void);
void Tecla_Reiniciar_Final(void);

#endif
