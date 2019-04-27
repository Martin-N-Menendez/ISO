#include "buttons.h"
#include "os.h"


volatile Buttons_t button_list[N_BUTTON];
gpioMap_t buttons_index[] = { TEC1,TEC2 };

void buttons_begin(void)
{
	int i=0;

	for( i = 0; i < N_BUTTON ; i++)
	{
		button_list[i].state = UP;
		button_list[i].pin = TEC1+i;
		button_list[i].begin_time = 0;
		button_list[i].end_time = 0;
	}
}

void tecla_presionada(gpioMap_t pin,Buttons_State estado)
{
	uint8_t index = buttons_index[pin-1];

	button_list[pin-1].state 							= estado;
	button_list[pin-1].pin     							= index;
	if (estado == UP)
		button_list[pin-1].begin_time					= get_tick_count();
	if (estado == DOWN)
		button_list[pin-1].end_time						= get_tick_count();
}

void Tecla_Reiniciar_Inicio(void)
{
	button_list[BUTTON1].begin_time = 0;
	button_list[BUTTON2].begin_time = 0;
}

void Tecla_Reiniciar_Final(void)
{
	button_list[BUTTON1].end_time = 0;
	button_list[BUTTON2].end_time = 0;
}
